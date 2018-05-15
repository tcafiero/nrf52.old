/**************************************************************************/
/*!
    @file     BLEGap.cpp
    @author   hathach

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2017, Adafruit Industries (adafruit.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include "bluefruit.h"

BLEGap::BLEGap(void)
{
  memclr(_peers, sizeof(_peers));

  _cfg_prph.mtu_max         = BLE_GATT_ATT_MTU_DEFAULT;
  _cfg_central.mtu_max      = BLE_GATT_ATT_MTU_DEFAULT;

#if SD_VER >= 500
  _cfg_prph.event_len       = BLE_GAP_EVENT_LENGTH_DEFAULT;
  _cfg_prph.hvn_tx_qsize    = BLE_GATTS_HVN_TX_QUEUE_SIZE_DEFAULT;
  _cfg_prph.wr_cmd_qsize    = BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT;

  _cfg_central.event_len    = BLE_GAP_EVENT_LENGTH_DEFAULT;
  _cfg_central.hvn_tx_qsize = BLE_GATTS_HVN_TX_QUEUE_SIZE_DEFAULT;
  _cfg_central.wr_cmd_qsize = BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT;
#endif

  _sec_param = (ble_gap_sec_params_t)
                {
                  .bond         = 1,
                  .mitm         = 0,
                  .lesc         = 0,
                  .keypress     = 0,
                  .io_caps      = BLE_GAP_IO_CAPS_NONE,
                  .oob          = 0,
                  .min_key_size = 7,
                  .max_key_size = 16,
                  .kdist_own    = { .enc = 1, .id = 1},
                  .kdist_peer   = { .enc = 1, .id = 1},
                };
}


void BLEGap::configPrphConn(uint16_t mtu_max, uint8_t event_len, uint8_t hvn_qsize, uint8_t wrcmd_qsize)
{
  _cfg_prph.mtu_max = maxof(mtu_max, BLE_GATT_ATT_MTU_DEFAULT);
#if SD_VER >= 500
  _cfg_prph.event_len = maxof(event_len, BLE_GAP_EVENT_LENGTH_MIN);
#endif
  _cfg_prph.hvn_tx_qsize = hvn_qsize;
  _cfg_prph.wr_cmd_qsize = wrcmd_qsize;
}

void BLEGap::configCentralConn(uint16_t mtu_max, uint8_t event_len, uint8_t hvn_qsize, uint8_t wrcmd_qsize)
{
  _cfg_central.mtu_max = maxof(mtu_max, BLE_GATT_ATT_MTU_DEFAULT);
#if SD_VER >= 500
  _cfg_central.event_len = maxof(event_len, BLE_GAP_EVENT_LENGTH_MIN);
#endif
  _cfg_central.hvn_tx_qsize = hvn_qsize;
  _cfg_central.wr_cmd_qsize = wrcmd_qsize;
}


uint16_t BLEGap::getMaxMtuByConnCfg(uint8_t conn_cfg)
{
  return (conn_cfg == CONN_CFG_PERIPHERAL) ? _cfg_prph.mtu_max : _cfg_central.mtu_max;
}

uint16_t BLEGap::getMaxMtu (uint8_t conn_hdl)
{
  return (getRole(conn_hdl) == BLE_GAP_ROLE_PERIPH) ? _cfg_prph.mtu_max : _cfg_central.mtu_max;
}

uint8_t BLEGap::getHvnQueueSize (uint8_t conn_hdl)
{
  return (getRole(conn_hdl) == BLE_GAP_ROLE_PERIPH) ? _cfg_prph.hvn_tx_qsize : _cfg_central.hvn_tx_qsize;
}

uint8_t BLEGap::getWriteCmdQueueSize (uint8_t conn_hdl)
{
  return (getRole(conn_hdl) == BLE_GAP_ROLE_PERIPH) ? _cfg_prph.wr_cmd_qsize : _cfg_central.wr_cmd_qsize;
}


/**
 * Get current Mac address and its type
 * @param mac address
 * @return Address type e.g BLE_GAP_ADDR_TYPE_RANDOM_STATIC
 */
uint8_t BLEGap::getAddr(uint8_t mac[6])
{
  ble_gap_addr_t addr;

#if SD_VER < 500
  sd_ble_gap_address_get(&addr);
#else
  sd_ble_gap_addr_get(&addr);
#endif

  memcpy(mac, addr.addr, 6);

  return addr.addr_type;
}

/**
 * Set the MAC address
 * @param mac   Bluetooth MAC Address
 * @param type  Must be either BLE_GAP_ADDR_TYPE_PUBLIC or BLE_GAP_ADDR_TYPE_RANDOM_STATIC
 * @return true if success
 */
bool BLEGap::setAddr(uint8_t mac[6], uint8_t type)
{
  ble_gap_addr_t addr;
  addr.addr_type = type;

  VERIFY (type == BLE_GAP_ADDR_TYPE_PUBLIC || type == BLE_GAP_ADDR_TYPE_RANDOM_STATIC);

  memcpy(addr.addr, mac, 6);

#if SD_VER < 500
  VERIFY_STATUS( sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &addr), false );
#else
  VERIFY_STATUS( sd_ble_gap_addr_set(&addr), false );
#endif
  return true;
}

bool BLEGap::connected(uint16_t conn_hdl)
{
  return _peers[conn_hdl].connected;
}

bool BLEGap::paired(uint16_t conn_hdl)
{
  return _peers[conn_hdl].paired;
}

bool BLEGap::requestPairing(uint16_t conn_hdl)
{
  gap_peer_t* peer = &_peers[conn_hdl];

  // skip if already paired
  if ( peer->paired ) return true;

  uint16_t cntr_ediv = 0xFFFF;

  if ( peer->role == BLE_GAP_ROLE_CENTRAL )
  {
    // Check to see if we did bonded with current prph previously
    bond_data_t bdata;

    if ( bond_find_cntr(&peer->addr, &bdata) )
    {
      cntr_ediv = bdata.peer_enc.master_id.ediv;
      LOG_LV2("BOND", "Load Keys from file " BOND_FNAME_CNTR, cntr_ediv);
      VERIFY_STATUS( sd_ble_gap_encrypt(conn_hdl, &bdata.peer_enc.master_id, &bdata.peer_enc.enc_info), false);

    }else
    {
      VERIFY_STATUS( sd_ble_gap_authenticate(conn_hdl, &_sec_param ), false);
    }
  }else
  {
    VERIFY_STATUS( sd_ble_gap_authenticate(conn_hdl, &_sec_param ), false);
  }

  // Wait for pairing process using on-the-fly semaphore
  peer->pair_sem = xSemaphoreCreateBinary();

  xSemaphoreTake(peer->pair_sem, portMAX_DELAY);

  // Failed to pair using centra stored keys, this happens when
  // Prph delete bonds while we did not --> let's remove the obsolete keyfile and move on
  if ( !peer->paired && (cntr_ediv != 0xffff) )
  {
    bond_remove_key(BLE_GAP_ROLE_CENTRAL, cntr_ediv);

    // Re-try with a fresh session
    VERIFY_STATUS( sd_ble_gap_authenticate(conn_hdl, &_sec_param ), false);

    xSemaphoreTake(peer->pair_sem, portMAX_DELAY);
  }

  vSemaphoreDelete(peer->pair_sem);
  peer->pair_sem = NULL;

  return peer->paired;
}

uint8_t BLEGap::getRole(uint16_t conn_hdl)
{
  return _peers[conn_hdl].role;
}

uint8_t BLEGap::getPeerAddr(uint16_t conn_hdl, uint8_t addr[6])
{
  memcpy(addr, _peers[conn_hdl].addr.addr, BLE_GAP_ADDR_LEN);
  return _peers[conn_hdl].addr.addr_type;
}

ble_gap_addr_t BLEGap::getPeerAddr(uint16_t conn_hdl)
{
  return _peers[conn_hdl].addr;
}

bool BLEGap::getHvnPacket(uint16_t conn_hdl)
{
  VERIFY( (conn_hdl < BLE_MAX_CONN) && (_peers[conn_hdl].hvn_tx_sem != NULL) );

  return xSemaphoreTake(_peers[conn_hdl].hvn_tx_sem, ms2tick(BLE_GENERIC_TIMEOUT));
}

bool BLEGap::getWriteCmdPacket(uint16_t conn_hdl)
{
#if SD_VER < 500
  return getHvnPacket(conn_hdl);
#else
  VERIFY( (conn_hdl < BLE_MAX_CONN) && (_peers[conn_hdl].wrcmd_tx_sem != NULL) );
  return xSemaphoreTake(_peers[conn_hdl].wrcmd_tx_sem, ms2tick(BLE_GENERIC_TIMEOUT));
#endif
}

uint16_t BLEGap::getMTU (uint16_t conn_hdl)
{
  return _peers[conn_hdl].att_mtu;
}

uint16_t BLEGap::getPeerName(uint16_t conn_hdl, char* buf, uint16_t bufsize)
{
  return Bluefruit.Gatt.readCharByUuid(conn_hdl, BLEUuid(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME), buf, bufsize);
}

/**
 * Event handler
 * @param evt
 */
void BLEGap::_eventHandler(ble_evt_t* evt)
{
  // conn handle has fixed offset regardless of event type
  const uint16_t conn_hdl = evt->evt.common_evt.conn_handle;

  gap_peer_t* peer = (conn_hdl == BLE_CONN_HANDLE_INVALID) ? NULL : &_peers[conn_hdl];

  switch(evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
    {
      ble_gap_evt_connected_t const * para = &evt->evt.gap_evt.params.connected;

      peer->connected = true;
      peer->role      = para->role;
      peer->addr      = para->peer_addr;
      peer->att_mtu   = BLE_GATT_ATT_MTU_DEFAULT;

      // Init transmission buffer for notification
      #if SD_VER < 500
      uint8_t txbuf_max;
      (void) sd_ble_tx_packet_count_get(conn_hdl, &txbuf_max);
      peer->hvn_tx_sem = xSemaphoreCreateCounting(txbuf_max, txbuf_max);
      #else
      peer->hvn_tx_sem   = xSemaphoreCreateCounting(getHvnQueueSize(conn_hdl), getHvnQueueSize(conn_hdl));
      #endif

      peer->wrcmd_tx_sem = xSemaphoreCreateCounting(getWriteCmdQueueSize(conn_hdl), getWriteCmdQueueSize(conn_hdl));
    }
    break;

    case BLE_GAP_EVT_DISCONNECTED:
    {
      ble_gap_evt_disconnected_t const* para = &evt->evt.gap_evt.params.disconnected;

      // mark as disconnected, but keep the role for sub sequence event handler
      peer->connected = peer->paired = false;

      vSemaphoreDelete( peer->hvn_tx_sem );
      peer->hvn_tx_sem = NULL;

      vSemaphoreDelete( peer->wrcmd_tx_sem );
      peer->wrcmd_tx_sem = NULL;
    }
    break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
    {
      // Pairing in progress, Peer asking for our info
      peer->bond_data = (bond_data_t*) rtos_malloc( sizeof(bond_data_t));
      VERIFY(peer->bond_data, );

      bond_data_t* bdata = peer->bond_data;
      memclr(bdata, sizeof(bond_data_t));

      peer->ediv = 0xFFFF; // invalid value for ediv

      /* Step 1: Pairing/Bonding
       * - Central supplies its parameters
       * - We replies with our security parameters
       */
      // ble_gap_sec_params_t* peer = &evt->evt.gap_evt.params.sec_params_request.peer_params;
      COMMENT_OUT(
          // Change security parameter according to authentication type
          if ( _auth_type == BLE_GAP_AUTH_KEY_TYPE_PASSKEY)
          {
            sec_para.mitm    = 1;
            sec_para.io_caps = BLE_GAP_IO_CAPS_DISPLAY_ONLY;
          }
      )

      ble_gap_sec_keyset_t keyset =
      {
          .keys_own = {
              .p_enc_key  = &bdata->own_enc,
              .p_id_key   = NULL,
              .p_sign_key = NULL,
              .p_pk       = NULL
          },

          .keys_peer = {
              .p_enc_key  = &bdata->peer_enc,
              .p_id_key   = &bdata->peer_id,
              .p_sign_key = NULL,
              .p_pk       = NULL
          }
      };

      VERIFY_STATUS(sd_ble_gap_sec_params_reply(conn_hdl,
                                                BLE_GAP_SEC_STATUS_SUCCESS,
                                                peer->role == BLE_GAP_ROLE_PERIPH ? &_sec_param : NULL,
                                                &keyset),
      );
    }
    break;

    case BLE_GAP_EVT_AUTH_STATUS:
    {
      // Bonding process completed
      ble_gap_evt_auth_status_t* status = &evt->evt.gap_evt.params.auth_status;

      // Pairing succeeded --> save encryption keys ( Bonding )
      if (BLE_GAP_SEC_STATUS_SUCCESS == status->auth_status)
      {
        peer->paired = true;
        peer->ediv   = peer->bond_data->own_enc.master_id.ediv;

        bond_save_keys(peer->role, conn_hdl, peer->bond_data);
      }else
      {
        PRINT_HEX(status->auth_status);
      }

      rtos_free(peer->bond_data);
      peer->bond_data = NULL;
    }
    break;

    case BLE_GAP_EVT_SEC_INFO_REQUEST:
    {
      // Reconnection. If bonded previously, Central will ask for stored keys.
      // return security information. Otherwise NULL
      ble_gap_evt_sec_info_request_t* sec_req = (ble_gap_evt_sec_info_request_t*) &evt->evt.gap_evt.params.sec_info_request;

      bond_data_t bdata;
      varclr(&bdata);

      if ( bond_load_keys(peer->role, sec_req->master_id.ediv, &bdata) )
      {
        sd_ble_gap_sec_info_reply(evt->evt.gap_evt.conn_handle, &bdata.own_enc.enc_info, &bdata.peer_id.id_info, NULL);

        peer->ediv   = bdata.own_enc.master_id.ediv;
      } else
      {
        sd_ble_gap_sec_info_reply(evt->evt.gap_evt.conn_handle, NULL, NULL, NULL);
      }
    }
    break;

    case BLE_GAP_EVT_CONN_SEC_UPDATE:
    {
      const ble_gap_conn_sec_t* conn_sec = &evt->evt.gap_evt.params.conn_sec_update.conn_sec;

      // Connection is secured (paired)
      if ( !( conn_sec->sec_mode.sm == 1 && conn_sec->sec_mode.lv == 1) )
      {
        // Previously bonded --> secure by re-connection process
        // --> Load & Set Sys Attr (Apply Service Context)
        // Else Init Sys Attr
        bond_load_cccd(peer->role, conn_hdl, peer->ediv);

        peer->paired = true;
      }

      if (peer->pair_sem) xSemaphoreGive(peer->pair_sem);
    }
    break;

    case BLE_GAP_EVT_PASSKEY_DISPLAY:
    {
      //      ble_gap_evt_passkey_display_t const* passkey_display = &evt->evt.gap_evt.params.passkey_display;
      //
      //      PRINT_INT(passkey_display->match_request);
      //      PRINT_BUFFER(passkey_display->passkey, 6);

      // sd_ble_gap_auth_key_reply
    }
    break;

#if SD_VER < 500
    case BLE_EVT_TX_COMPLETE:
      if ( peer->hvn_tx_sem )
      {
        for(uint8_t i=0; i<evt->evt.common_evt.params.tx_complete.count; i++)
        {
          xSemaphoreGive(peer->hvn_tx_sem);
        }
      }
    break;

#else
    case BLE_GATTS_EVT_HVN_TX_COMPLETE:
      if ( peer->hvn_tx_sem )
      {
        for(uint8_t i=0; i<evt->evt.gatts_evt.params.hvn_tx_complete.count; i++)
        {
          xSemaphoreGive(peer->hvn_tx_sem);
        }
      }
    break;

    case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE:
      if ( peer->wrcmd_tx_sem )
      {
        for(uint8_t i=0; i<evt->evt.gattc_evt.params.write_cmd_tx_complete.count; i++)
        {
          xSemaphoreGive(peer->wrcmd_tx_sem);
        }
      }
    break;

    case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
    {
      ble_gap_data_length_params_t* param = &evt->evt.gap_evt.params.data_length_update_request.peer_params;
      LOG_LV2("GAP", "Data Length Req is (tx, rx) octets = (%d, %d), (tx, rx) time = (%d, %d) us",
              param->max_tx_octets, param->max_rx_octets, param->max_tx_time_us, param->max_rx_time_us);

      // Let Softdevice decide the data length
      VERIFY_STATUS( sd_ble_gap_data_length_update(conn_hdl, NULL, NULL), );
    }
    break;

    case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
    {
      ble_gap_data_length_params_t* datalen =  &evt->evt.gap_evt.params.data_length_update.effective_params;
      LOG_LV2("GAP", "Data Length is (tx, rx) octets = (%d, %d), (tx, rx) time = (%d, %d) us",
                   datalen->max_tx_octets, datalen->max_rx_octets, datalen->max_tx_time_us, datalen->max_rx_time_us);
    }
    break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
      ble_gap_phys_t* req_phy = &evt->evt.gap_evt.params.phy_update_request.peer_preferred_phys;

      #if CFG_DEBUG >= 1
      char const *phy_str[] = { "Auto", "1 Mbps", "2 Mbps", "Coded" };
      LOG_LV1("GAP", "PHY request tx: %s, rx: %s", phy_str[req_phy->tx_phys], phy_str[req_phy->rx_phys]);
      #endif

      // Tell SoftDevice to choose PHY automatically
      ble_gap_phys_t phy = { BLE_GAP_PHY_AUTO, BLE_GAP_PHY_AUTO };
      (void) sd_ble_gap_phy_update(conn_hdl, &phy);
    }
    break;

    case BLE_GAP_EVT_PHY_UPDATE:
    {
      ble_gap_evt_phy_update_t* active_phy = &evt->evt.gap_evt.params.phy_update;

      #if CFG_DEBUG >= 1
      if ( active_phy->status != BLE_HCI_STATUS_CODE_SUCCESS )
      {
        LOG_LV1("GAP", "Failed HCI status = 0x%02X", active_phy->status);
      }else
      {
        char const *phy_str[] = { "Auto", "1 Mbps", "2 Mbps", "Coded" };
        LOG_LV1("GAP", "PHY active tx: %s, rx: %s", phy_str[active_phy->tx_phy], phy_str[active_phy->rx_phy]);
      }
      #endif
    }
    break;

    case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
    {
      peer->att_mtu = minof(evt->evt.gatts_evt.params.exchange_mtu_request.client_rx_mtu, getMaxMtu(conn_hdl));
      VERIFY_STATUS( sd_ble_gatts_exchange_mtu_reply(conn_hdl, peer->att_mtu), );

      LOG_LV1("GAP", "ATT MTU is changed to %d", peer->att_mtu);
    }
    break;

#endif

    default: break;
  }
}
