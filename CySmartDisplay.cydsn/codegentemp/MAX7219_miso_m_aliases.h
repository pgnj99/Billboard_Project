/*******************************************************************************
* File Name: MAX7219_miso_m.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_MAX7219_miso_m_ALIASES_H) /* Pins MAX7219_miso_m_ALIASES_H */
#define CY_PINS_MAX7219_miso_m_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define MAX7219_miso_m_0			(MAX7219_miso_m__0__PC)
#define MAX7219_miso_m_0_PS		(MAX7219_miso_m__0__PS)
#define MAX7219_miso_m_0_PC		(MAX7219_miso_m__0__PC)
#define MAX7219_miso_m_0_DR		(MAX7219_miso_m__0__DR)
#define MAX7219_miso_m_0_SHIFT	(MAX7219_miso_m__0__SHIFT)
#define MAX7219_miso_m_0_INTR	((uint16)((uint16)0x0003u << (MAX7219_miso_m__0__SHIFT*2u)))

#define MAX7219_miso_m_INTR_ALL	 ((uint16)(MAX7219_miso_m_0_INTR))


#endif /* End Pins MAX7219_miso_m_ALIASES_H */


/* [] END OF FILE */
