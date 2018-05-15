/**************************************************************************/
/*!
    @file     HAPCharacteristic.h
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
#ifndef HAPCHARACTERISTIC_H_
#define HAPCHARACTERISTIC_H_

#include <BLECharacteristic.h>
#include "HAPProcedure.h"

enum HAPChrProperties_t
{
  HAP_CHR_PROPS_READ                  = bit(0),
  HAP_CHR_PROPS_WRITE                 = bit(1),
  HAP_CHR_PROPS_ADDITIONAL_AUTH_DATA  = bit(2),
  HAP_CHR_PROPS_TIMED_WRITE_PROCEDURE = bit(3),
  HAP_CHR_PROPS_SECURE_READ           = bit(4),
  HAP_CHR_PROPS_SECURE_WRITE          = bit(5),
  HAP_CHR_PROPS_HIDDEN                = bit(6), // from user
  HAP_CHR_PROPS_NOTIFY                = bit(7),
  HAP_CHR_PROPS_NOTIFY_DISCONNECTED   = bit(8),
};

class HAPCharacteristic : public BLECharacteristic
{
  public:
    typedef HAPResponse_t* (*hap_write_cb_t) (HAPCharacteristic& chr, ble_gatts_evt_write_t const* gatt_req, HAPRequest_t const* hap_req);
    static BLEUuid _g_uuid_cid;

    HAPCharacteristic(BLEUuid bleuuid, uint8_t format, uint16_t unit = UUID16_UNIT_UNITLESS);
    virtual err_t begin(void);

    void setHapProperties(uint16_t prop);

    // Write to Hap Value
    uint16_t writeHapValue(const void* data, uint16_t len);
    uint16_t writeHapValue(const char* str);
    uint16_t writeHapValue(uint32_t num);

    // Callbacks
    void setHapWriteCallback(hap_write_cb_t fp);

    HAPResponse_t* createHapResponse(uint8_t tid, uint8_t status, TLV8_t tlv_para[] = NULL, uint8_t count = 0);

    /*------------- Internal Functions -------------*/
    virtual void _eventHandler(ble_evt_t* event);

  private:
    uint16_t _cid;
    uint16_t _hap_props;

    uint16_t _resp_len;

    // Char value is read by HAP procedure, not exposed via GATT
    void*    _value;
    uint16_t _vallen;

    // Callbacks
    hap_write_cb_t _hap_wr_cb;

    err_t _addChrIdDescriptor(void);

    HAPResponse_t* processChrSignatureRead(HAPRequest_t* hap_req);
    HAPResponse_t* processChrRead(HAPRequest_t* hap_req);

};

#endif /* HAPCHARACTERISTIC_H_ */
