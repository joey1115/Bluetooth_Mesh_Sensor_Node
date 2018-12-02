#include "m_BLE.h"

static ret_code_t m_ble_advertising_update(ble_advertising_t *const p_advertising, ble_advertising_init_t const *const p_init);
static void pm_evt_handler(pm_evt_t const *p_evt);
static void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt);
static void nrf_qwr_error_handler(uint32_t nrf_error);
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt);
static void conn_params_error_handler(uint32_t nrf_error);
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);

#ifdef DEBUG
BLE_BAS_DEF(m_bas); /**< Structure used to identify the battery service. */
#endif

NRF_BLE_GATT_DEF(m_gatt);           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising); /**< Advertising module instance. */

static ble_uuid_t m_adv_uuids[] = /**< Universally unique service identifiers. */
    {
#ifdef DEBUG
        {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE},
#endif
        {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}};
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static ble_advdata_manuf_data_t m_advdata_manuf_data;
static uint8_t bas_seat_data[NUM_OF_ADV_BYTE];
static ble_advertising_init_t advertising_data_init;

void update_adv_temp(uint16_t temp)
{
    ret_code_t error_code;

    advertising_data_init.advdata.p_manuf_specific_data->data.p_data[2] = temp & 0xFF;
    advertising_data_init.advdata.p_manuf_specific_data->data.p_data[3] = (temp>>8) & 0xFF;
    error_code = m_ble_advertising_update(&m_advertising, &advertising_data_init);
    APP_ERROR_CHECK(error_code);
}

void update_adv_bal(uint8_t bal)
{
    ret_code_t error_code;

    advertising_data_init.advdata.p_manuf_specific_data->data.p_data[0] = bal;
    error_code = m_ble_advertising_update(&m_advertising, &advertising_data_init);
    APP_ERROR_CHECK(error_code);
}

void update_adv_seat(uint8_t seat)
{
    ret_code_t error_code;

    advertising_data_init.advdata.p_manuf_specific_data->data.p_data[1] = seat;
    error_code = m_ble_advertising_update(&m_advertising, &advertising_data_init);
    APP_ERROR_CHECK(error_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds)
{
    ret_code_t err_code;

    if (erase_bonds == true)
    {
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
        err_code = pm_peers_delete();
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}

#ifdef DEBUG
/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
void battery_level_update(uint8_t battery_level)
{
    ret_code_t err_code;

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
    {
        APP_ERROR_HANDLER(err_code);
    }
}
#endif

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void gap_params_init(void)
{
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
void services_init(void)
{
    ret_code_t err_code;

#ifdef DEBUG
    ble_bas_init_t bas_init;
#endif

    ble_dis_init_t dis_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

#ifdef DEBUG
    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    bas_init.evt_handler = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref = NULL;
    bas_init.initial_batt_level = 100;

    // Here the sec level for the Battery Service can be changed/increased.
    bas_init.bl_rd_sec = SEC_OPEN;
    bas_init.bl_cccd_wr_sec = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);
#endif
    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for the Peer Manager initialization.
 */
void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    ret_code_t err_code;
    // ble_advertising_init_t init;

    memset(&advertising_data_init, 0, sizeof(advertising_data_init));

    advertising_data_init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    advertising_data_init.advdata.include_appearance = false;
    advertising_data_init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advertising_data_init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advertising_data_init.advdata.uuids_complete.p_uuids = m_adv_uuids;
    bas_seat_data[0] = 0;
    bas_seat_data[1] = 0;
    m_advdata_manuf_data.company_identifier = 0x0A;
    m_advdata_manuf_data.data.size = sizeof(bas_seat_data) / sizeof(bas_seat_data[0]);
    m_advdata_manuf_data.data.p_data = bas_seat_data;
    advertising_data_init.advdata.p_manuf_specific_data = &m_advdata_manuf_data;

    advertising_data_init.config.ble_adv_fast_enabled = true;
    advertising_data_init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    advertising_data_init.config.ble_adv_fast_timeout = APP_ADV_DURATION;

    advertising_data_init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &advertising_data_init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

ret_code_t m_nrf_ble_lesc_request_handler(void)
{
    return (nrf_ble_lesc_request_handler());
}

static ret_code_t m_ble_advertising_update(ble_advertising_t *const p_advertising, ble_advertising_init_t const *const p_init)
{
    uint32_t ret;
    if ((p_init == NULL) || (p_advertising == NULL))
    {
        return NRF_ERROR_NULL;
    }
    p_advertising->adv_data.adv_data.p_data = p_advertising->enc_advdata;

    if (p_advertising->adv_modes_config.ble_adv_extended_enabled == true)
    {
#ifdef BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED
        p_advertising->adv_data.adv_data.len = BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED;
#else
        p_advertising->adv_data.adv_data.len = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
#endif // BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED
    }
    else
    {
        p_advertising->adv_data.adv_data.len = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
    }

    ret = ble_advdata_encode(&p_init->advdata, p_advertising->enc_advdata, &p_advertising->adv_data.adv_data.len);
    VERIFY_SUCCESS(ret);

    if (&p_init->srdata != NULL)
    {
        p_advertising->adv_data.scan_rsp_data.p_data = p_advertising->enc_scan_rsp_data;
        if (p_advertising->adv_modes_config.ble_adv_extended_enabled == true)
        {
#ifdef BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED
            p_advertising->adv_data.scan_rsp_data.len = BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED;
#else
            p_advertising->adv_data.scan_rsp_data.len = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
#endif // BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED
        }
        else
        {
            p_advertising->adv_data.scan_rsp_data.len = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
        }
        ret = ble_advdata_encode(&p_init->srdata, p_advertising->adv_data.scan_rsp_data.p_data, &p_advertising->adv_data.scan_rsp_data.len);
        VERIFY_SUCCESS(ret);
    }
    else
    {
        p_advertising->adv_data.scan_rsp_data.p_data = NULL;
        p_advertising->adv_data.scan_rsp_data.len = 0;
    }
    return ret;
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
    case PM_EVT_PEERS_DELETE_SUCCEEDED:
        advertising_start(false);
        break;

    default:
        break;
    }
}

/**@brief GATT module event handler.
 */
static void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)
    {
#ifdef DEBUG
        NRF_LOG_INFO("GATT ATT MTU on connection 0x%x changed to %d.",
                     p_evt->conn_handle,
                     p_evt->params.att_mtu_effective);
#endif
    }
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;
    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
#ifdef DEBUG
        NRF_LOG_INFO("Fast advertising.");
#endif
        break;
    case BLE_ADV_EVT_IDLE:
        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
        break;

        // case BLE_ADV_EVT_DIRECTED_HIGH_DUTY:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;

        // case BLE_ADV_EVT_DIRECTED:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;

    case BLE_ADV_EVT_SLOW:
        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
        break;

        // case BLE_ADV_EVT_FAST_WHITELIST:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;

        // case BLE_ADV_EVT_SLOW_WHITELIST:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;

        // case BLE_ADV_EVT_WHITELIST_REQUEST:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;

        // case BLE_ADV_EVT_PEER_ADDR_REQUEST:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;

        // default:
        //     err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        //     APP_ERROR_CHECK(err_code);
        //     break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
#ifdef DEBUG
        NRF_LOG_INFO("Connected.");
#endif
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_DISCONNECTED:

#ifdef DEBUG
        NRF_LOG_INFO("Disconnected, reason %d.",
                     p_ble_evt->evt.gap_evt.params.disconnected.reason);
#endif
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
#ifdef DEBUG
        NRF_LOG_DEBUG("PHY update request.");
#endif
        ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.

#ifdef DEBUG
        NRF_LOG_DEBUG("GATT Client Timeout.");
#endif
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
#ifdef DEBUG
        NRF_LOG_DEBUG("GATT Server Timeout.");
#endif
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
#ifdef DEBUG
        NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
#endif
        break;

    case BLE_GAP_EVT_AUTH_KEY_REQUEST:
#ifdef DEBUG
        NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST");
#endif
        break;

    case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
#ifdef DEBUG
        NRF_LOG_INFO("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
#endif
        break;

    case BLE_GAP_EVT_AUTH_STATUS:
#ifdef DEBUG
        NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
                     p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                     p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                     p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                     *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                     *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
#endif
        break;

    default:
        // No implementation needed.
        break;
    }
}
