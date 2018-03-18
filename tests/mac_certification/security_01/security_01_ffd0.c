/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE: TP/154/MAC/SECURITY-01. P_Coord - FFD0
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_mac.h"

#define USE_ZB_MLME_START_CONFIRM
#define USE_ZB_MCPS_DATA_CONFIRM
#define USE_ZB_MLME_ASSOCIATE_INDICATION
#define USE_ZB_MCPS_DATA_INDICATION
#include "zb_mac_only_stubs.h"


/*! \addtogroup ZB_TESTS */
/*! @{ */

#include "zb_bank_common.h"

#define CHANNEL 0x16
#define PAN_ID 0x1aaa
static zb_ieee_addr_t g_zc_addr = {0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xde, 0xac};
static zb_ieee_addr_t g_rfd_addr = {0x02, 0x00, 0x00, 0x00, 0x00, 0x48, 0xde, 0xac};

//static zb_ieee_addr_t g_rfd_addr = {0xac, 0xde, 0x48, 0x00, 0x00, 0x00, 0x00, 0x02};
//static zb_ieee_addr_t g_zc_addr = {0xac, 0xde, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01};

//static zb_uint8_t g_key[16] = {0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8, 0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0 };

static zb_uint8_t g_key[16] = {0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf };

static zb_uint8_t g_msdu[23] = {0x7f, 0x33, 0xf2, 0xb1, 0x00, 0x02, 0x74, 0x5a, 0x53, 0x65, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x73, 0x61, 0x79, 0x73, 0x20, 0x48, 0x69, 0x21};


MAIN()
{
  ARGV_UNUSED;

  ZB_INIT("security_01_ffd0", argv[1], argv[2]);
/*
  Security: no for all the devices
  Channel: 0x14 for all the devices
  PAN id: 0x1AAA for all the devices
  MAC: 0xACDE480000000001
*/

  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_zc_addr);
  MAC_PIB().mac_pan_id = PAN_ID;
  ZB_PIB_SHORT_ADDRESS() = 0x1122;
  ZB_MEMCPY(MAC_PIB().mac_key, g_key, 16);
  MAC_PIB().mac_dsn = 0;
  MAC_PIB().mac_frame_counter = 1;
  MAC_PIB().mac_device_table_entries = 1;
  MAC_PIB().mac_device_table[0].short_address = 0x3344;
  ZB_IEEE_ADDR_COPY(MAC_PIB().mac_device_table[0].long_address, g_rfd_addr);
  MAC_PIB().mac_device_table[0].frame_counter = 1;
  MAC_PIB().mac_device_table[0].pan_id = PAN_ID;
  /* start */
  {
    zb_buf_t *buf = zb_get_out_buf();

    zb_mlme_start_req_t *req = ZB_GET_BUF_PARAM(buf, zb_mlme_start_req_t);

    ZB_BZERO(req, sizeof(*req));
    req->pan_id = MAC_PIB().mac_pan_id;
    req->logical_channel = CHANNEL;
    req->pan_coordinator = 1;      /* will be coordinator */
    req->beacon_order = ZB_TURN_OFF_ORDER;
    req->superframe_order = ZB_TURN_OFF_ORDER;

    ZB_SCHEDULE_CALLBACK(zb_mlme_start_request, ZB_REF_FROM_BUF(buf));
  }

  while (1)
  {
    zb_sched_loop_iteration();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}


void zb_mlme_start_confirm(zb_uint8_t param) ZB_CALLBACK
{
  TRACE_MSG(TRACE_NWK2, "zb_mlme_start_confirm", (FMT__0));

  zb_free_buf(ZB_BUF_FROM_REF(param));
}


void zb_mlme_associate_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_ieee_addr_t device_address;
  zb_mlme_associate_indication_t *request = ZB_GET_BUF_PARAM((zb_buf_t *)ZB_BUF_FROM_REF(param), zb_mlme_associate_indication_t);
  TRACE_MSG(TRACE_NWK1, ">>mlme_associate_ind %hd", (FMT__H, param));
  /*
    Very simple implementation: accept anybody, assign address 0x3344
   */
  ZB_IEEE_ADDR_COPY(device_address, request->device_address);

  ZB_MLME_BUILD_ASSOCIATE_RESPONSE(ZB_BUF_FROM_REF(param), device_address, 0x3344, 0);

  ZB_SCHEDULE_CALLBACK(zb_mlme_associate_response, param);

  TRACE_MSG(TRACE_NWK1, "<<mlme_associate_ind", (FMT__0));
}


void zb_mcps_data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_mac_mhr_t mac_hdr;
  zb_uint8_t mhr_len;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *p;
  TRACE_MSG(TRACE_MAC1, ">> zb_mcps_data_indication param %hd", (FMT__H, param));

  mhr_len = zb_parse_mhr(&mac_hdr, ZB_BUF_BEGIN(buf));
  ZB_MAC_CUT_HDR(buf, mhr_len, p);

  TRACE_MSG(TRACE_MAC1, "in data len %hd security_level %hd key_id_mode %hd, key_index %hd",
            (FMT__H_H_H_H, ZB_BUF_LEN(buf),
             mac_hdr.security_level, mac_hdr.key_id_mode, mac_hdr.key_index));
  
  {
    zb_ushort_t i;
    for (i = 0 ; i < ZB_BUF_LEN(buf) ; ++i)
    {
      TRACE_MSG(TRACE_MAC1, " [%hd] 0x%hx", (FMT__H_H, i, p[i]));
    }
  }

  {
    zb_uint8_t *pl;
    ZB_BUF_INITIAL_ALLOC(ZB_BUF_FROM_REF(param), 23, pl);
    ZB_MEMCPY(pl, g_msdu, 23);
  }
  {
    zb_mcps_data_req_params_t *data_req = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mcps_data_req_params_t);

    data_req->dst_addr.addr_short = 0x3344;
    data_req->src_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    data_req->src_addr.addr_short = 0x1122;
    data_req->dst_addr_mode = ZB_ADDR_16BIT_DEV_OR_BROADCAST;
    data_req->dst_pan_id = PAN_ID;
    data_req->msdu_handle = 0xc;
    data_req->tx_options = 0;
    data_req->security_level = 5;
    data_req->key_id_mode = 1;
    data_req->key_index = 1;
  }

  MAC_PIB().mac_dsn = 0;

  ZB_SCHEDULE_CALLBACK(zb_mcps_data_request, param);
}


void zb_mcps_data_confirm(zb_uint8_t param) ZB_CALLBACK
{
  zb_mcps_data_confirm_params_t *confirm_params = ZB_GET_BUF_PARAM(ZB_BUF_FROM_REF(param), zb_mcps_data_confirm_params_t);
  TRACE_MSG(TRACE_NWK2, "zb_mcps_data_confirm param %hd handle 0x%hx status 0x%hx",
            (FMT__H_H_H, (zb_uint8_t)param, (zb_uint8_t)confirm_params->msdu_handle,
             (zb_uint8_t)ZB_BUF_FROM_REF(param)->u.hdr.status));
  zb_free_buf(ZB_BUF_FROM_REF(param));
}


/*! @} */
