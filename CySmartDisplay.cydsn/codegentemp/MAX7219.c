/***************************************************************************//**
* \file MAX7219.c
* \version 4.0
*
* \brief
*  This file provides the source code to the API for the SCB Component.
*
* Note:
*
*******************************************************************************
* \copyright
* Copyright 2013-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "MAX7219_PVT.h"

#if (MAX7219_SCB_MODE_I2C_INC)
    #include "MAX7219_I2C_PVT.h"
#endif /* (MAX7219_SCB_MODE_I2C_INC) */

#if (MAX7219_SCB_MODE_EZI2C_INC)
    #include "MAX7219_EZI2C_PVT.h"
#endif /* (MAX7219_SCB_MODE_EZI2C_INC) */

#if (MAX7219_SCB_MODE_SPI_INC || MAX7219_SCB_MODE_UART_INC)
    #include "MAX7219_SPI_UART_PVT.h"
#endif /* (MAX7219_SCB_MODE_SPI_INC || MAX7219_SCB_MODE_UART_INC) */


/***************************************
*    Run Time Configuration Vars
***************************************/

/* Stores internal component configuration for Unconfigured mode */
#if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    uint8 MAX7219_scbMode = MAX7219_SCB_MODE_UNCONFIG;
    uint8 MAX7219_scbEnableWake;
    uint8 MAX7219_scbEnableIntr;

    /* I2C configuration variables */
    uint8 MAX7219_mode;
    uint8 MAX7219_acceptAddr;

    /* SPI/UART configuration variables */
    volatile uint8 * MAX7219_rxBuffer;
    uint8  MAX7219_rxDataBits;
    uint32 MAX7219_rxBufferSize;

    volatile uint8 * MAX7219_txBuffer;
    uint8  MAX7219_txDataBits;
    uint32 MAX7219_txBufferSize;

    /* EZI2C configuration variables */
    uint8 MAX7219_numberOfAddr;
    uint8 MAX7219_subAddrSize;
#endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Common SCB Vars
***************************************/
/**
* \addtogroup group_general
* \{
*/

/** MAX7219_initVar indicates whether the MAX7219 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time SCB_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the MAX7219_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  MAX7219_Init() function can be called before the 
*  MAX7219_Start() or MAX7219_Enable() function.
*/
uint8 MAX7219_initVar = 0u;


#if (! (MAX7219_SCB_MODE_I2C_CONST_CFG || \
        MAX7219_SCB_MODE_EZI2C_CONST_CFG))
    /** This global variable stores TX interrupt sources after 
    * MAX7219_Stop() is called. Only these TX interrupt sources 
    * will be restored on a subsequent MAX7219_Enable() call.
    */
    uint16 MAX7219_IntrTxMask = 0u;
#endif /* (! (MAX7219_SCB_MODE_I2C_CONST_CFG || \
              MAX7219_SCB_MODE_EZI2C_CONST_CFG)) */
/** \} globals */

#if (MAX7219_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_MAX7219_CUSTOM_INTR_HANDLER)
    void (*MAX7219_customIntrHandler)(void) = NULL;
#endif /* !defined (CY_REMOVE_MAX7219_CUSTOM_INTR_HANDLER) */
#endif /* (MAX7219_SCB_IRQ_INTERNAL) */


/***************************************
*    Private Function Prototypes
***************************************/

static void MAX7219_ScbEnableIntr(void);
static void MAX7219_ScbModeStop(void);
static void MAX7219_ScbModePostEnable(void);


/*******************************************************************************
* Function Name: MAX7219_Init
****************************************************************************//**
*
*  Initializes the MAX7219 component to operate in one of the selected
*  configurations: I2C, SPI, UART or EZI2C.
*  When the configuration is set to "Unconfigured SCB", this function does
*  not do any initialization. Use mode-specific initialization APIs instead:
*  MAX7219_I2CInit, MAX7219_SpiInit, 
*  MAX7219_UartInit or MAX7219_EzI2CInit.
*
*******************************************************************************/
void MAX7219_Init(void)
{
#if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
    if (MAX7219_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        MAX7219_initVar = 0u;
    }
    else
    {
        /* Initialization was done before this function call */
    }

#elif (MAX7219_SCB_MODE_I2C_CONST_CFG)
    MAX7219_I2CInit();

#elif (MAX7219_SCB_MODE_SPI_CONST_CFG)
    MAX7219_SpiInit();

#elif (MAX7219_SCB_MODE_UART_CONST_CFG)
    MAX7219_UartInit();

#elif (MAX7219_SCB_MODE_EZI2C_CONST_CFG)
    MAX7219_EzI2CInit();

#endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MAX7219_Enable
****************************************************************************//**
*
*  Enables MAX7219 component operation: activates the hardware and 
*  internal interrupt. It also restores TX interrupt sources disabled after the 
*  MAX7219_Stop() function was called (note that level-triggered TX 
*  interrupt sources remain disabled to not cause code lock-up).
*  For I2C and EZI2C modes the interrupt is internal and mandatory for 
*  operation. For SPI and UART modes the interrupt can be configured as none, 
*  internal or external.
*  The MAX7219 configuration should be not changed when the component
*  is enabled. Any configuration changes should be made after disabling the 
*  component.
*  When configuration is set to “Unconfigured MAX7219”, the component 
*  must first be initialized to operate in one of the following configurations: 
*  I2C, SPI, UART or EZ I2C, using the mode-specific initialization API. 
*  Otherwise this function does not enable the component.
*
*******************************************************************************/
void MAX7219_Enable(void)
{
#if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Enable SCB block, only if it is already configured */
    if (!MAX7219_SCB_MODE_UNCONFIG_RUNTM_CFG)
    {
        MAX7219_CTRL_REG |= MAX7219_CTRL_ENABLED;

        MAX7219_ScbEnableIntr();

        /* Call PostEnable function specific to current operation mode */
        MAX7219_ScbModePostEnable();
    }
#else
    MAX7219_CTRL_REG |= MAX7219_CTRL_ENABLED;

    MAX7219_ScbEnableIntr();

    /* Call PostEnable function specific to current operation mode */
    MAX7219_ScbModePostEnable();
#endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MAX7219_Start
****************************************************************************//**
*
*  Invokes MAX7219_Init() and MAX7219_Enable().
*  After this function call, the component is enabled and ready for operation.
*  When configuration is set to "Unconfigured SCB", the component must first be
*  initialized to operate in one of the following configurations: I2C, SPI, UART
*  or EZI2C. Otherwise this function does not enable the component.
*
* \globalvars
*  MAX7219_initVar - used to check initial configuration, modified
*  on first function call.
*
*******************************************************************************/
void MAX7219_Start(void)
{
    if (0u == MAX7219_initVar)
    {
        MAX7219_Init();
        MAX7219_initVar = 1u; /* Component was initialized */
    }

    MAX7219_Enable();
}


/*******************************************************************************
* Function Name: MAX7219_Stop
****************************************************************************//**
*
*  Disables the MAX7219 component: disable the hardware and internal 
*  interrupt. It also disables all TX interrupt sources so as not to cause an 
*  unexpected interrupt trigger because after the component is enabled, the 
*  TX FIFO is empty.
*  Refer to the function MAX7219_Enable() for the interrupt 
*  configuration details.
*  This function disables the SCB component without checking to see if 
*  communication is in progress. Before calling this function it may be 
*  necessary to check the status of communication to make sure communication 
*  is complete. If this is not done then communication could be stopped mid 
*  byte and corrupted data could result.
*
*******************************************************************************/
void MAX7219_Stop(void)
{
#if (MAX7219_SCB_IRQ_INTERNAL)
    MAX7219_DisableInt();
#endif /* (MAX7219_SCB_IRQ_INTERNAL) */

    /* Call Stop function specific to current operation mode */
    MAX7219_ScbModeStop();

    /* Disable SCB IP */
    MAX7219_CTRL_REG &= (uint32) ~MAX7219_CTRL_ENABLED;

    /* Disable all TX interrupt sources so as not to cause an unexpected
    * interrupt trigger after the component will be enabled because the 
    * TX FIFO is empty.
    * For SCB IP v0, it is critical as it does not mask-out interrupt
    * sources when it is disabled. This can cause a code lock-up in the
    * interrupt handler because TX FIFO cannot be loaded after the block
    * is disabled.
    */
    MAX7219_SetTxInterruptMode(MAX7219_NO_INTR_SOURCES);

#if (MAX7219_SCB_IRQ_INTERNAL)
    MAX7219_ClearPendingInt();
#endif /* (MAX7219_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: MAX7219_SetRxFifoLevel
****************************************************************************//**
*
*  Sets level in the RX FIFO to generate a RX level interrupt.
*  When the RX FIFO has more entries than the RX FIFO level an RX level
*  interrupt request is generated.
*
*  \param level: Level in the RX FIFO to generate RX level interrupt.
*   The range of valid level values is between 0 and RX FIFO depth - 1.
*
*******************************************************************************/
void MAX7219_SetRxFifoLevel(uint32 level)
{
    uint32 rxFifoCtrl;

    rxFifoCtrl = MAX7219_RX_FIFO_CTRL_REG;

    rxFifoCtrl &= ((uint32) ~MAX7219_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    rxFifoCtrl |= ((uint32) (MAX7219_RX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    MAX7219_RX_FIFO_CTRL_REG = rxFifoCtrl;
}


/*******************************************************************************
* Function Name: MAX7219_SetTxFifoLevel
****************************************************************************//**
*
*  Sets level in the TX FIFO to generate a TX level interrupt.
*  When the TX FIFO has less entries than the TX FIFO level an TX level
*  interrupt request is generated.
*
*  \param level: Level in the TX FIFO to generate TX level interrupt.
*   The range of valid level values is between 0 and TX FIFO depth - 1.
*
*******************************************************************************/
void MAX7219_SetTxFifoLevel(uint32 level)
{
    uint32 txFifoCtrl;

    txFifoCtrl = MAX7219_TX_FIFO_CTRL_REG;

    txFifoCtrl &= ((uint32) ~MAX7219_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK); /* Clear level mask bits */
    txFifoCtrl |= ((uint32) (MAX7219_TX_FIFO_CTRL_TRIGGER_LEVEL_MASK & level));

    MAX7219_TX_FIFO_CTRL_REG = txFifoCtrl;
}


#if (MAX7219_SCB_IRQ_INTERNAL)
    /*******************************************************************************
    * Function Name: MAX7219_SetCustomInterruptHandler
    ****************************************************************************//**
    *
    *  Registers a function to be called by the internal interrupt handler.
    *  First the function that is registered is called, then the internal interrupt
    *  handler performs any operation such as software buffer management functions
    *  before the interrupt returns.  It is the user's responsibility not to break
    *  the software buffer operations. Only one custom handler is supported, which
    *  is the function provided by the most recent call.
    *  At the initialization time no custom handler is registered.
    *
    *  \param func: Pointer to the function to register.
    *        The value NULL indicates to remove the current custom interrupt
    *        handler.
    *
    *******************************************************************************/
    void MAX7219_SetCustomInterruptHandler(void (*func)(void))
    {
    #if !defined (CY_REMOVE_MAX7219_CUSTOM_INTR_HANDLER)
        MAX7219_customIntrHandler = func; /* Register interrupt handler */
    #else
        if (NULL != func)
        {
            /* Suppress compiler warning */
        }
    #endif /* !defined (CY_REMOVE_MAX7219_CUSTOM_INTR_HANDLER) */
    }
#endif /* (MAX7219_SCB_IRQ_INTERNAL) */


/*******************************************************************************
* Function Name: MAX7219_ScbModeEnableIntr
****************************************************************************//**
*
*  Enables an interrupt for a specific mode.
*
*******************************************************************************/
static void MAX7219_ScbEnableIntr(void)
{
#if (MAX7219_SCB_IRQ_INTERNAL)
    /* Enable interrupt in NVIC */
    #if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
        if (0u != MAX7219_scbEnableIntr)
        {
            MAX7219_EnableInt();
        }

    #else
        MAX7219_EnableInt();

    #endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */
#endif /* (MAX7219_SCB_IRQ_INTERNAL) */
}


/*******************************************************************************
* Function Name: MAX7219_ScbModePostEnable
****************************************************************************//**
*
*  Calls the PostEnable function for a specific operation mode.
*
*******************************************************************************/
static void MAX7219_ScbModePostEnable(void)
{
#if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
#if (!MAX7219_CY_SCBIP_V1)
    if (MAX7219_SCB_MODE_SPI_RUNTM_CFG)
    {
        MAX7219_SpiPostEnable();
    }
    else if (MAX7219_SCB_MODE_UART_RUNTM_CFG)
    {
        MAX7219_UartPostEnable();
    }
    else
    {
        /* Unknown mode: do nothing */
    }
#endif /* (!MAX7219_CY_SCBIP_V1) */

#elif (MAX7219_SCB_MODE_SPI_CONST_CFG)
    MAX7219_SpiPostEnable();

#elif (MAX7219_SCB_MODE_UART_CONST_CFG)
    MAX7219_UartPostEnable();

#else
    /* Unknown mode: do nothing */
#endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */
}


/*******************************************************************************
* Function Name: MAX7219_ScbModeStop
****************************************************************************//**
*
*  Calls the Stop function for a specific operation mode.
*
*******************************************************************************/
static void MAX7219_ScbModeStop(void)
{
#if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
    if (MAX7219_SCB_MODE_I2C_RUNTM_CFG)
    {
        MAX7219_I2CStop();
    }
    else if (MAX7219_SCB_MODE_EZI2C_RUNTM_CFG)
    {
        MAX7219_EzI2CStop();
    }
#if (!MAX7219_CY_SCBIP_V1)
    else if (MAX7219_SCB_MODE_SPI_RUNTM_CFG)
    {
        MAX7219_SpiStop();
    }
    else if (MAX7219_SCB_MODE_UART_RUNTM_CFG)
    {
        MAX7219_UartStop();
    }
#endif /* (!MAX7219_CY_SCBIP_V1) */
    else
    {
        /* Unknown mode: do nothing */
    }
#elif (MAX7219_SCB_MODE_I2C_CONST_CFG)
    MAX7219_I2CStop();

#elif (MAX7219_SCB_MODE_EZI2C_CONST_CFG)
    MAX7219_EzI2CStop();

#elif (MAX7219_SCB_MODE_SPI_CONST_CFG)
    MAX7219_SpiStop();

#elif (MAX7219_SCB_MODE_UART_CONST_CFG)
    MAX7219_UartStop();

#else
    /* Unknown mode: do nothing */
#endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */
}


#if (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG)
    /*******************************************************************************
    * Function Name: MAX7219_SetPins
    ****************************************************************************//**
    *
    *  Sets the pins settings accordingly to the selected operation mode.
    *  Only available in the Unconfigured operation mode. The mode specific
    *  initialization function calls it.
    *  Pins configuration is set by PSoC Creator when a specific mode of operation
    *  is selected in design time.
    *
    *  \param mode:      Mode of SCB operation.
    *  \param subMode:   Sub-mode of SCB operation. It is only required for SPI and UART
    *             modes.
    *  \param uartEnableMask: enables TX or RX direction and RTS and CTS signals.
    *
    *******************************************************************************/
    void MAX7219_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask)
    {
        uint32 pinsDm[MAX7219_SCB_PINS_NUMBER];
        uint32 i;
        
    #if (!MAX7219_CY_SCBIP_V1)
        uint32 pinsInBuf = 0u;
    #endif /* (!MAX7219_CY_SCBIP_V1) */
        
        uint32 hsiomSel[MAX7219_SCB_PINS_NUMBER] = 
        {
            MAX7219_RX_SDA_MOSI_HSIOM_SEL_GPIO,
            MAX7219_TX_SCL_MISO_HSIOM_SEL_GPIO,
            0u,
            0u,
            0u,
            0u,
            0u,
        };

    #if (MAX7219_CY_SCBIP_V1)
        /* Supress compiler warning. */
        if ((0u == subMode) || (0u == uartEnableMask))
        {
        }
    #endif /* (MAX7219_CY_SCBIP_V1) */

        /* Set default HSIOM to GPIO and Drive Mode to Analog Hi-Z */
        for (i = 0u; i < MAX7219_SCB_PINS_NUMBER; i++)
        {
            pinsDm[i] = MAX7219_PIN_DM_ALG_HIZ;
        }

        if ((MAX7219_SCB_MODE_I2C   == mode) ||
            (MAX7219_SCB_MODE_EZI2C == mode))
        {
        #if (MAX7219_RX_SDA_MOSI_PIN)
            hsiomSel[MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_RX_SDA_MOSI_HSIOM_SEL_I2C;
            pinsDm  [MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_PIN_DM_OD_LO;
        #elif (MAX7219_RX_WAKE_SDA_MOSI_PIN)
            hsiomSel[MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX] = MAX7219_RX_WAKE_SDA_MOSI_HSIOM_SEL_I2C;
            pinsDm  [MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX] = MAX7219_PIN_DM_OD_LO;
        #else
        #endif /* (MAX7219_RX_SDA_MOSI_PIN) */
        
        #if (MAX7219_TX_SCL_MISO_PIN)
            hsiomSel[MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_TX_SCL_MISO_HSIOM_SEL_I2C;
            pinsDm  [MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_PIN_DM_OD_LO;
        #endif /* (MAX7219_TX_SCL_MISO_PIN) */
        }
    #if (!MAX7219_CY_SCBIP_V1)
        else if (MAX7219_SCB_MODE_SPI == mode)
        {
        #if (MAX7219_RX_SDA_MOSI_PIN)
            hsiomSel[MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_RX_SDA_MOSI_HSIOM_SEL_SPI;
        #elif (MAX7219_RX_WAKE_SDA_MOSI_PIN)
            hsiomSel[MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX] = MAX7219_RX_WAKE_SDA_MOSI_HSIOM_SEL_SPI;
        #else
        #endif /* (MAX7219_RX_SDA_MOSI_PIN) */
        
        #if (MAX7219_TX_SCL_MISO_PIN)
            hsiomSel[MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_TX_SCL_MISO_HSIOM_SEL_SPI;
        #endif /* (MAX7219_TX_SCL_MISO_PIN) */
        
        #if (MAX7219_CTS_SCLK_PIN)
            hsiomSel[MAX7219_CTS_SCLK_PIN_INDEX] = MAX7219_CTS_SCLK_HSIOM_SEL_SPI;
        #endif /* (MAX7219_CTS_SCLK_PIN) */

            if (MAX7219_SPI_SLAVE == subMode)
            {
                /* Slave */
                pinsDm[MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;
                pinsDm[MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                pinsDm[MAX7219_CTS_SCLK_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;

            #if (MAX7219_RTS_SS0_PIN)
                /* Only SS0 is valid choice for Slave */
                hsiomSel[MAX7219_RTS_SS0_PIN_INDEX] = MAX7219_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm  [MAX7219_RTS_SS0_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;
            #endif /* (MAX7219_RTS_SS0_PIN) */

            #if (MAX7219_TX_SCL_MISO_PIN)
                /* Disable input buffer */
                 pinsInBuf |= MAX7219_TX_SCL_MISO_PIN_MASK;
            #endif /* (MAX7219_TX_SCL_MISO_PIN) */
            }
            else 
            {
                /* (Master) */
                pinsDm[MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                pinsDm[MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;
                pinsDm[MAX7219_CTS_SCLK_PIN_INDEX] = MAX7219_PIN_DM_STRONG;

            #if (MAX7219_RTS_SS0_PIN)
                hsiomSel [MAX7219_RTS_SS0_PIN_INDEX] = MAX7219_RTS_SS0_HSIOM_SEL_SPI;
                pinsDm   [MAX7219_RTS_SS0_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                pinsInBuf |= MAX7219_RTS_SS0_PIN_MASK;
            #endif /* (MAX7219_RTS_SS0_PIN) */

            #if (MAX7219_SS1_PIN)
                hsiomSel [MAX7219_SS1_PIN_INDEX] = MAX7219_SS1_HSIOM_SEL_SPI;
                pinsDm   [MAX7219_SS1_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                pinsInBuf |= MAX7219_SS1_PIN_MASK;
            #endif /* (MAX7219_SS1_PIN) */

            #if (MAX7219_SS2_PIN)
                hsiomSel [MAX7219_SS2_PIN_INDEX] = MAX7219_SS2_HSIOM_SEL_SPI;
                pinsDm   [MAX7219_SS2_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                pinsInBuf |= MAX7219_SS2_PIN_MASK;
            #endif /* (MAX7219_SS2_PIN) */

            #if (MAX7219_SS3_PIN)
                hsiomSel [MAX7219_SS3_PIN_INDEX] = MAX7219_SS3_HSIOM_SEL_SPI;
                pinsDm   [MAX7219_SS3_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                pinsInBuf |= MAX7219_SS3_PIN_MASK;
            #endif /* (MAX7219_SS3_PIN) */

                /* Disable input buffers */
            #if (MAX7219_RX_SDA_MOSI_PIN)
                pinsInBuf |= MAX7219_RX_SDA_MOSI_PIN_MASK;
            #elif (MAX7219_RX_WAKE_SDA_MOSI_PIN)
                pinsInBuf |= MAX7219_RX_WAKE_SDA_MOSI_PIN_MASK;
            #else
            #endif /* (MAX7219_RX_SDA_MOSI_PIN) */

            #if (MAX7219_CTS_SCLK_PIN)
                pinsInBuf |= MAX7219_CTS_SCLK_PIN_MASK;
            #endif /* (MAX7219_CTS_SCLK_PIN) */
            }
        }
        else /* UART */
        {
            if (MAX7219_UART_MODE_SMARTCARD == subMode)
            {
                /* SmartCard */
            #if (MAX7219_TX_SCL_MISO_PIN)
                hsiomSel[MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_TX_SCL_MISO_HSIOM_SEL_UART;
                pinsDm  [MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_PIN_DM_OD_LO;
            #endif /* (MAX7219_TX_SCL_MISO_PIN) */
            }
            else /* Standard or IrDA */
            {
                if (0u != (MAX7219_UART_RX_PIN_ENABLE & uartEnableMask))
                {
                #if (MAX7219_RX_SDA_MOSI_PIN)
                    hsiomSel[MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_RX_SDA_MOSI_HSIOM_SEL_UART;
                    pinsDm  [MAX7219_RX_SDA_MOSI_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;
                #elif (MAX7219_RX_WAKE_SDA_MOSI_PIN)
                    hsiomSel[MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX] = MAX7219_RX_WAKE_SDA_MOSI_HSIOM_SEL_UART;
                    pinsDm  [MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;
                #else
                #endif /* (MAX7219_RX_SDA_MOSI_PIN) */
                }

                if (0u != (MAX7219_UART_TX_PIN_ENABLE & uartEnableMask))
                {
                #if (MAX7219_TX_SCL_MISO_PIN)
                    hsiomSel[MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_TX_SCL_MISO_HSIOM_SEL_UART;
                    pinsDm  [MAX7219_TX_SCL_MISO_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                    
                    /* Disable input buffer */
                    pinsInBuf |= MAX7219_TX_SCL_MISO_PIN_MASK;
                #endif /* (MAX7219_TX_SCL_MISO_PIN) */
                }

            #if !(MAX7219_CY_SCBIP_V0 || MAX7219_CY_SCBIP_V1)
                if (MAX7219_UART_MODE_STD == subMode)
                {
                    if (0u != (MAX7219_UART_CTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* CTS input is multiplexed with SCLK */
                    #if (MAX7219_CTS_SCLK_PIN)
                        hsiomSel[MAX7219_CTS_SCLK_PIN_INDEX] = MAX7219_CTS_SCLK_HSIOM_SEL_UART;
                        pinsDm  [MAX7219_CTS_SCLK_PIN_INDEX] = MAX7219_PIN_DM_DIG_HIZ;
                    #endif /* (MAX7219_CTS_SCLK_PIN) */
                    }

                    if (0u != (MAX7219_UART_RTS_PIN_ENABLE & uartEnableMask))
                    {
                        /* RTS output is multiplexed with SS0 */
                    #if (MAX7219_RTS_SS0_PIN)
                        hsiomSel[MAX7219_RTS_SS0_PIN_INDEX] = MAX7219_RTS_SS0_HSIOM_SEL_UART;
                        pinsDm  [MAX7219_RTS_SS0_PIN_INDEX] = MAX7219_PIN_DM_STRONG;
                        
                        /* Disable input buffer */
                        pinsInBuf |= MAX7219_RTS_SS0_PIN_MASK;
                    #endif /* (MAX7219_RTS_SS0_PIN) */
                    }
                }
            #endif /* !(MAX7219_CY_SCBIP_V0 || MAX7219_CY_SCBIP_V1) */
            }
        }
    #endif /* (!MAX7219_CY_SCBIP_V1) */

    /* Configure pins: set HSIOM, DM and InputBufEnable */
    /* Note: the DR register settings do not effect the pin output if HSIOM is other than GPIO */

    #if (MAX7219_RX_SDA_MOSI_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_RX_SDA_MOSI_HSIOM_REG,
                                       MAX7219_RX_SDA_MOSI_HSIOM_MASK,
                                       MAX7219_RX_SDA_MOSI_HSIOM_POS,
                                        hsiomSel[MAX7219_RX_SDA_MOSI_PIN_INDEX]);

        MAX7219_uart_rx_i2c_sda_spi_mosi_SetDriveMode((uint8) pinsDm[MAX7219_RX_SDA_MOSI_PIN_INDEX]);

        #if (!MAX7219_CY_SCBIP_V1)
            MAX7219_SET_INP_DIS(MAX7219_uart_rx_i2c_sda_spi_mosi_INP_DIS,
                                         MAX7219_uart_rx_i2c_sda_spi_mosi_MASK,
                                         (0u != (pinsInBuf & MAX7219_RX_SDA_MOSI_PIN_MASK)));
        #endif /* (!MAX7219_CY_SCBIP_V1) */
    
    #elif (MAX7219_RX_WAKE_SDA_MOSI_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_RX_WAKE_SDA_MOSI_HSIOM_REG,
                                       MAX7219_RX_WAKE_SDA_MOSI_HSIOM_MASK,
                                       MAX7219_RX_WAKE_SDA_MOSI_HSIOM_POS,
                                       hsiomSel[MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX]);

        MAX7219_uart_rx_wake_i2c_sda_spi_mosi_SetDriveMode((uint8)
                                                               pinsDm[MAX7219_RX_WAKE_SDA_MOSI_PIN_INDEX]);

        MAX7219_SET_INP_DIS(MAX7219_uart_rx_wake_i2c_sda_spi_mosi_INP_DIS,
                                     MAX7219_uart_rx_wake_i2c_sda_spi_mosi_MASK,
                                     (0u != (pinsInBuf & MAX7219_RX_WAKE_SDA_MOSI_PIN_MASK)));

         /* Set interrupt on falling edge */
        MAX7219_SET_INCFG_TYPE(MAX7219_RX_WAKE_SDA_MOSI_INTCFG_REG,
                                        MAX7219_RX_WAKE_SDA_MOSI_INTCFG_TYPE_MASK,
                                        MAX7219_RX_WAKE_SDA_MOSI_INTCFG_TYPE_POS,
                                        MAX7219_INTCFG_TYPE_FALLING_EDGE);
    #else
    #endif /* (MAX7219_RX_WAKE_SDA_MOSI_PIN) */

    #if (MAX7219_TX_SCL_MISO_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_TX_SCL_MISO_HSIOM_REG,
                                       MAX7219_TX_SCL_MISO_HSIOM_MASK,
                                       MAX7219_TX_SCL_MISO_HSIOM_POS,
                                        hsiomSel[MAX7219_TX_SCL_MISO_PIN_INDEX]);

        MAX7219_uart_tx_i2c_scl_spi_miso_SetDriveMode((uint8) pinsDm[MAX7219_TX_SCL_MISO_PIN_INDEX]);

    #if (!MAX7219_CY_SCBIP_V1)
        MAX7219_SET_INP_DIS(MAX7219_uart_tx_i2c_scl_spi_miso_INP_DIS,
                                     MAX7219_uart_tx_i2c_scl_spi_miso_MASK,
                                    (0u != (pinsInBuf & MAX7219_TX_SCL_MISO_PIN_MASK)));
    #endif /* (!MAX7219_CY_SCBIP_V1) */
    #endif /* (MAX7219_RX_SDA_MOSI_PIN) */

    #if (MAX7219_CTS_SCLK_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_CTS_SCLK_HSIOM_REG,
                                       MAX7219_CTS_SCLK_HSIOM_MASK,
                                       MAX7219_CTS_SCLK_HSIOM_POS,
                                       hsiomSel[MAX7219_CTS_SCLK_PIN_INDEX]);

        MAX7219_uart_cts_spi_sclk_SetDriveMode((uint8) pinsDm[MAX7219_CTS_SCLK_PIN_INDEX]);

        MAX7219_SET_INP_DIS(MAX7219_uart_cts_spi_sclk_INP_DIS,
                                     MAX7219_uart_cts_spi_sclk_MASK,
                                     (0u != (pinsInBuf & MAX7219_CTS_SCLK_PIN_MASK)));
    #endif /* (MAX7219_CTS_SCLK_PIN) */

    #if (MAX7219_RTS_SS0_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_RTS_SS0_HSIOM_REG,
                                       MAX7219_RTS_SS0_HSIOM_MASK,
                                       MAX7219_RTS_SS0_HSIOM_POS,
                                       hsiomSel[MAX7219_RTS_SS0_PIN_INDEX]);

        MAX7219_uart_rts_spi_ss0_SetDriveMode((uint8) pinsDm[MAX7219_RTS_SS0_PIN_INDEX]);

        MAX7219_SET_INP_DIS(MAX7219_uart_rts_spi_ss0_INP_DIS,
                                     MAX7219_uart_rts_spi_ss0_MASK,
                                     (0u != (pinsInBuf & MAX7219_RTS_SS0_PIN_MASK)));
    #endif /* (MAX7219_RTS_SS0_PIN) */

    #if (MAX7219_SS1_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_SS1_HSIOM_REG,
                                       MAX7219_SS1_HSIOM_MASK,
                                       MAX7219_SS1_HSIOM_POS,
                                       hsiomSel[MAX7219_SS1_PIN_INDEX]);

        MAX7219_spi_ss1_SetDriveMode((uint8) pinsDm[MAX7219_SS1_PIN_INDEX]);

        MAX7219_SET_INP_DIS(MAX7219_spi_ss1_INP_DIS,
                                     MAX7219_spi_ss1_MASK,
                                     (0u != (pinsInBuf & MAX7219_SS1_PIN_MASK)));
    #endif /* (MAX7219_SS1_PIN) */

    #if (MAX7219_SS2_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_SS2_HSIOM_REG,
                                       MAX7219_SS2_HSIOM_MASK,
                                       MAX7219_SS2_HSIOM_POS,
                                       hsiomSel[MAX7219_SS2_PIN_INDEX]);

        MAX7219_spi_ss2_SetDriveMode((uint8) pinsDm[MAX7219_SS2_PIN_INDEX]);

        MAX7219_SET_INP_DIS(MAX7219_spi_ss2_INP_DIS,
                                     MAX7219_spi_ss2_MASK,
                                     (0u != (pinsInBuf & MAX7219_SS2_PIN_MASK)));
    #endif /* (MAX7219_SS2_PIN) */

    #if (MAX7219_SS3_PIN)
        MAX7219_SET_HSIOM_SEL(MAX7219_SS3_HSIOM_REG,
                                       MAX7219_SS3_HSIOM_MASK,
                                       MAX7219_SS3_HSIOM_POS,
                                       hsiomSel[MAX7219_SS3_PIN_INDEX]);

        MAX7219_spi_ss3_SetDriveMode((uint8) pinsDm[MAX7219_SS3_PIN_INDEX]);

        MAX7219_SET_INP_DIS(MAX7219_spi_ss3_INP_DIS,
                                     MAX7219_spi_ss3_MASK,
                                     (0u != (pinsInBuf & MAX7219_SS3_PIN_MASK)));
    #endif /* (MAX7219_SS3_PIN) */
    }

#endif /* (MAX7219_SCB_MODE_UNCONFIG_CONST_CFG) */


#if (MAX7219_CY_SCBIP_V0 || MAX7219_CY_SCBIP_V1)
    /*******************************************************************************
    * Function Name: MAX7219_I2CSlaveNackGeneration
    ****************************************************************************//**
    *
    *  Sets command to generate NACK to the address or data.
    *
    *******************************************************************************/
    void MAX7219_I2CSlaveNackGeneration(void)
    {
        /* Check for EC_AM toggle condition: EC_AM and clock stretching for address are enabled */
        if ((0u != (MAX7219_CTRL_REG & MAX7219_CTRL_EC_AM_MODE)) &&
            (0u == (MAX7219_I2C_CTRL_REG & MAX7219_I2C_CTRL_S_NOT_READY_ADDR_NACK)))
        {
            /* Toggle EC_AM before NACK generation */
            MAX7219_CTRL_REG &= ~MAX7219_CTRL_EC_AM_MODE;
            MAX7219_CTRL_REG |=  MAX7219_CTRL_EC_AM_MODE;
        }

        MAX7219_I2C_SLAVE_CMD_REG = MAX7219_I2C_SLAVE_CMD_S_NACK;
    }
#endif /* (MAX7219_CY_SCBIP_V0 || MAX7219_CY_SCBIP_V1) */


/* [] END OF FILE */
