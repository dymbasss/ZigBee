/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE: TP/APS/BV-09 ZC/ZR- APS TX Multiple Data Frame
Version for test with Daintree devices: no visibility limit by long MAC address.
*/

#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

zb_ieee_addr_t g_ieee_addr = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
zb_ieee_addr_t g_zr_ieee_addr = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
zb_ieee_addr_t daintree_1 = {0x00, 0x00, 0x6B, 0x00, 0x00, 0x81, 0x22, 0x00};
zb_ieee_addr_t daintree_2 = {0x00, 0x00, 0x6C, 0x00, 0x00, 0x81, 0x22, 0x00};


#ifndef ZB_NS_BUILD
#define HW_SLEEP_ADDITIONAL 30
#else
#define HW_SLEEP_ADDITIONAL 0
#endif

MAIN()
{
  ARGV_UNUSED;

#ifndef KEIL
  if ( argc < 3 )
  {
    printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_zc", argv[1], argv[2]);
#else
  ZB_INIT("zdo_zc", "1", "1");
#endif

  /* let's always be coordinator */
  ZB_AIB().aps_designated_coordinator = 1;
  MAC_PIB().mac_pan_id = 0x1aaa;
  /* set ieee addr */
//  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ieee_addr);
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &daintree_1);

  /* accept only one child */
  ZB_NWK().max_children = 1;

  MAC_ADD_VISIBLE_LONG(g_zr_ieee_addr);
  

#ifndef ZB_NS_BUILD
  ZB_UPDATE_LONGMAC();
  ZB_UPDATE_PAN_ID();
#endif

#ifdef ZB_SECURITY
  /* turn off security */
  ZB_NIB_SECURITY_LEVEL() = 0;
#endif

  if ( zdo_dev_start() != RET_OK )
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}

#define T_DATA_SIZE 0x0a
/* data size to be transfered via counted packte */
void test_buffer_request(zb_uint8_t param)
{
  zb_buf_t *buf = zb_get_out_buf();
  zb_apsde_data_req_t *dreq = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  zb_tp_transmit_counted_packets_req_t *req;
  zb_uint8_t i;
  zb_uint8_t *ptr;

  ZVUNUSED(param);

  ZB_MEMSET(dreq, 0, sizeof(*dreq));
  ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_tp_transmit_counted_packets_req_t), req);
  req->len = T_DATA_SIZE;
  req->counter = 0x0;
  ZB_BUF_ALLOC_RIGHT(buf, T_DATA_SIZE, ptr);
  for (i = 0; i<T_DATA_SIZE;i++)
  {
   ptr[i] = i%32+'0';
  }
  dreq->profileid = ZB_TEST_PROFILE_ID;
  dreq->clusterid = TP_TRANSMIT_COUNTED_PACKETS_CLID;
  dreq->dst_endpoint = 0xf0;
  dreq->src_endpoint = 0x01;
  dreq->dst_addr.addr_short = 0xffff;
  dreq->tx_options = 0;
  dreq->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  dreq->radius = 7;
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}

void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);

  TRACE_MSG(TRACE_ERROR, ">>zb_zdo_startup_complete status %d", (FMT__D, (int)buf->u.hdr.status));

  if (buf->u.hdr.status == RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "Device STARTED OK", (FMT__0));
    ZB_SCHEDULE_ALARM(test_buffer_request, 0, (30 + HW_SLEEP_ADDITIONAL) * ZB_TIME_ONE_SECOND);
    ZB_SCHEDULE_ALARM(test_buffer_request, 0, (32 + HW_SLEEP_ADDITIONAL) * ZB_TIME_ONE_SECOND);
    ZB_SCHEDULE_ALARM(test_buffer_request, 0, (34 + HW_SLEEP_ADDITIONAL) * ZB_TIME_ONE_SECOND);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device START FAILED", (FMT__0));
  }

  zb_free_buf(buf);
}
