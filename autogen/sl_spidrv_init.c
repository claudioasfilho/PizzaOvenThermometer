#include "spidrv.h"
#include "sl_spidrv_instances.h"

#include "sl_spidrv_MAX31865_config.h"

SPIDRV_HandleData_t sl_spidrv_MAX31865_handle_data;
SPIDRV_Handle_t sl_spidrv_MAX31865_handle = &sl_spidrv_MAX31865_handle_data;

SPIDRV_Init_t sl_spidrv_init_MAX31865 = {
  .port = SL_SPIDRV_MAX31865_PERIPHERAL,
#if defined(_USART_ROUTELOC0_MASK)
  .portLocationTx = SL_SPIDRV_MAX31865_TX_LOC,
  .portLocationRx = SL_SPIDRV_MAX31865_RX_LOC,
  .portLocationClk = SL_SPIDRV_MAX31865_CLK_LOC,
  .portLocationCs = SL_SPIDRV_MAX31865_CS_LOC,
#elif defined(_GPIO_USART_ROUTEEN_MASK)
  .portTx = SL_SPIDRV_MAX31865_TX_PORT,
  .portRx = SL_SPIDRV_MAX31865_RX_PORT,
  .portClk = SL_SPIDRV_MAX31865_CLK_PORT,
  .portCs = SL_SPIDRV_MAX31865_CS_PORT,
  .pinTx = SL_SPIDRV_MAX31865_TX_PIN,
  .pinRx = SL_SPIDRV_MAX31865_RX_PIN,
  .pinClk = SL_SPIDRV_MAX31865_CLK_PIN,
  .pinCs = SL_SPIDRV_MAX31865_CS_PIN,
#else
  .portLocation = SL_SPIDRV_MAX31865_ROUTE_LOC,
#endif
  .bitRate = SL_SPIDRV_MAX31865_BITRATE,
  .frameLength = SL_SPIDRV_MAX31865_FRAME_LENGTH,
  .dummyTxValue = 0,
  .type = SL_SPIDRV_MAX31865_TYPE,
  .bitOrder = SL_SPIDRV_MAX31865_BIT_ORDER,
  .clockMode = SL_SPIDRV_MAX31865_CLOCK_MODE,
  .csControl = SL_SPIDRV_MAX31865_CS_CONTROL,
  .slaveStartMode = SL_SPIDRV_MAX31865_SLAVE_START_MODE,
};

void sl_spidrv_init_instances(void) {
  SPIDRV_Init(sl_spidrv_MAX31865_handle, &sl_spidrv_init_MAX31865);
}
