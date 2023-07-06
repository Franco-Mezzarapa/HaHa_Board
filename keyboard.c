#include "Keyboard.h"
#include <util/delay.h>
#include <avr/sleep.h>


/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber              = INTERFACE_ID_Keyboard,
		.ReportINEndpoint             =
		{
			.Address              = KEYBOARD_EPADDR,
			.Size                 = KEYBOARD_EPSIZE,
			.Banks                = 1,
		},
		.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
		.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
	},
};

//Open powershell.
//initialize this array with NULL. We're going to use a terminator to break loops.
// This is going to prevent the array from going on longer than we want.
uint8_t scanCodes[100] = {NULL};
uint8_t modifierKeys[100] = {NULL};
	
int executeKeys(void){
	
	//Invoke the callback function.
	// Create and populate HID report data
	// Manually invoke the callback function

	uint8_t reportID = 0;
	uint8_t reportType = HID_REPORT_ITEM_In;
	uint8_t reportData[8]; // Example report data
	uint16_t reportSize = sizeof(reportData);
	
	//Send HID_report for windows search menu
	scanCodes[0] = HID_KEYBOARD_SC_R;
	modifierKeys[0] = HID_KEYBOARD_MODIFIER_LEFTGUI;
	CALLBACK_HID_Device_CreateHIDReport(NULL,reportID,reportType,reportData,reportSize);
	  _delay_ms(5000); // Delay between keypresses
	
	//Send CMD :)
	modifierKeys[0] = NULL;
	scanCodes[0] = HID_KEYBOARD_SC_C;
	scanCodes[1] = HID_KEYBOARD_SC_M;
	scanCodes[2] = HID_KEYBOARD_SC_D;
	scanCodes[3] = HID_KEYBOARD_SC_ENTER;
	CALLBACK_HID_Device_CreateHIDReport(NULL,reportID,reportType,reportData,reportSize);
	 _delay_ms(5000); // Delay between keypresses
	
	
	int scanLenght = sizeof(scanCodes)/sizeof(scanCodes[0]);
	for(int i = 0; i >scanLenght;i++){
		scanCodes[i] = NULL;
	}

	

}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */

int main(void)
{
	bool result;
	SetupHardware();

	GlobalInterruptEnable();
	    // Initialize USB stack
	    USB_Init();

		
		//Executes the keystrokes sequentially!!
		executeKeys();
		


	for (;;)
	{
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();		
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	Buttons_Init();
	USB_Init();
}



/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	USB_Device_EnableSOFEvents();

}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
    uint8_t numKeyCodes = sizeof(scanCodes) / sizeof(scanCodes[0]);
	int modCounter = 0;
	int nullCounter = 0;
	
		while (modifierKeys[modCounter] != NULL)
		{
			KeyboardReport->Modifier = modifierKeys[modCounter];
			modCounter++;
		}
		
		while (scanCodes[nullCounter] != NULL)
		{
			KeyboardReport->KeyCode[nullCounter] = scanCodes[nullCounter];
			nullCounter++;
		}
			
	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return true;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
}



