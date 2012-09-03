#include <avr/pgmspace.h>
#include <LUFA/Drivers/USB/USB.h>
#include "spiout.h"

typedef struct {
    USB_Descriptor_Configuration_Header_t Config;
    USB_Descriptor_Interface_t Interface;
    USB_Descriptor_Endpoint_t DataEp;
} USB_Descriptor_Configuration_t;

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
	.USBSpecification       = VERSION_BCD(01.10),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,
	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
	.VendorID               = 0x03EB,
	.ProductID              = 0x204F,
	.ReleaseNumber          = VERSION_BCD(00.01),
	.ManufacturerStrIndex   = 0x01,
	.ProductStrIndex        = 0x02,
	.SerialNumStrIndex      = NO_DESCRIPTOR,
	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
	.Config = {
        .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},
        .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
        .TotalInterfaces        = 1,
        .ConfigurationNumber    = 1,
        .ConfigurationStrIndex  = NO_DESCRIPTOR,
        .ConfigAttributes       = (USB_CONFIG_ATTR_BUSPOWERED),
        .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
    },
    .Interface = {
        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
        .InterfaceNumber        = 0x00,
        .AlternateSetting       = 0x00,
        .TotalEndpoints         = 1,
        .Class                  = 0xff,
        .SubClass               = 0x00,
        .Protocol               = 0x00,
        .InterfaceStrIndex      = NO_DESCRIPTOR
    },
	.DataEp = {
		.Header					= {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
		.EndpointAddress		= ENDPOINT_DIR_OUT | DATA_EPNUM,
		.Attributes				= EP_TYPE_ISOCHRONOUS | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
		.EndpointSize			= DATA_EPSIZE,
		.PollingIntervalMS		= 0x01
	}
};

const USB_Descriptor_String_t PROGMEM LanguageString = {
	.Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},
	.UnicodeString          = {LANGUAGE_ID_ENG}
};

const USB_Descriptor_String_t PROGMEM ManufacturerString = {
	.Header                 = {.Size = USB_STRING_LEN(13), .Type = DTYPE_String},
	.UnicodeString          = L"Angus McInnes"
};

const USB_Descriptor_String_t PROGMEM ProductString = {
	.Header                 = {.Size = USB_STRING_LEN(13), .Type = DTYPE_String},
	.UnicodeString          = L"USB commander"
};

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case 0x00:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case 0x01:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case 0x02:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
			}

			break;
	}

	*DescriptorAddress = Address;
	return Size;
}
