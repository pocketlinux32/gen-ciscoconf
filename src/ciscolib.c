/***********************************\
* Cisco Config Generator, v0.56     *
* (c)2021 pocketlinux32, Under GPL3 *
* Types Source File                 *
\***********************************/
#include <ciscolib-const.h>
#include <ciscolib.h>

// Cisco Interface Structure
struct ciscoint {
	ciscoconst_t type;
	ciscoconst_t mode;
	uint16_t ports[2];
	char description[4096];
	plarray_t* allowedVlans;
	char ipAddr[44];
	uint16_t subMask;
	char gateway[44];
}

// Cisco Table Structure
struct ciscotable {
	ciscoconst_t type;
	ciscoconst_t mode;
	char name[128];
	plarray_t* interfaces;
}

// Allocates memory for an interface structure and returns it
ciscoint_t* ciscoCreateInterface(ciscoconst_t type, uint16_t port1, uint16_t port2, plgc_t* gc){
	ciscoint_t* returnInt = plGCAlloc(gc, sizeof(ciscoint_t));

	returnInt->type = type;
	returnInt->mode = CISCO_MODE_ACCESS;
	returnInt->ports[0] = port1;
	returnInt->ports[1] = port2;
	returnInt->allowedVlans = plGCAlloc(gc, sizeof(plarray_t));
	returnInt->allowedVlans->size = 0;
	returnInt->allowedVlans->array = plGCAlloc(gc, 2 * sizeof(uint16_t));
	returnInt->ipAddr = "";
	returnInt->subMask = 0;
	returnInt->gateway = "";

	return returnInt;
}

// Allocates memory for a table structure and returns it
ciscotable_t* ciscoCreateTable(ciscoconst_t type, ciscoconst_t mode, plgc_t* gc){
	ciscotable_t* returnTable = plGCAlloc(gc, sizeof(ciscotable_t));

	returnTable->type = type;
	returnTable->mode = mode;
	returnTable->interfaces = plGCAlloc(gc, plarray_t);
	returnTable->interfaces->size = 0;
	returnTable->interfaces->array = plGCAlloc(gc, 2 * sizeof(ciscoint_t*));

	return returnTable;
}

// Modifies attributes in an interface
uint8_t ciscoModifyInterface(ciscoint_t* interface, plgc_t* gc, ciscoconst_t modType, ...){
	va_list values;
	va_start(values, modType);
	ciscoconst_t constant;
	char* string;
	uint16_t numbers[2];

	// Type parser
	switch(modType){
		case CISCO_MODTYPE_TYPE: ;
		case CISCO_MODTYPE_MODE: ;
			constant = va_arg(values, ciscoconst_t);
			break;
		case CISCO_MODTYPE_DESC: ;
		case CISCO_MODTYPE_IP_ADDR: ;
		case CISCO_MODTYPE_GATEWAY: ;
			string = va_arg(values, char*);
			break;
		case CISCO_MODTYPE_PORTS: ;
			numbers[0] = va_arg(values, uint16_t);
		case CISCO_MODTYPE_ALLOW_VLAN: ;
		case CISCO_MODTYPE_SUBMASK: ;
			numbers[1] = va_arg(values, uint16_t);
			break;
		default:
			return CISCO_ERROR_INVALID_ACTION;
	}

	// Action Parser
	switch(modType){
		case CISCO_MODTYPE_TYPE: ;
			if(constant > CISCO_INT_VLAN)
				return CISCO_ERROR_INVALID_VALUE;

			interface->type = constant;
			break;
		case CISCO_MODTYPE_MODE: ;
			if(interface->mode == CISCO_MODE_IN_PORTCH){
				return CISCO_ERROR_INVALID_ACTION;
			}else if(constant < CISCO_MODE_ACCESS || constant > CISCO_MODE_IN_PORTCH){
				return CISCO_ERROR_INVALID_VALUE;
			}

			interface->mode = constant;
			break;
		case CISCO_MODTYPE_PORTS: ;
			interface->ports[0] = numbers[0];
			interface->ports[1] = numbers[1];
			break;
		case CISCO_MODTYPE_DESC: ;
			if(strlen(string)+1 > 4096)
				return CISCO_ERROR_BUFFER_OVERFLOW;

			strcpy(interface->description, string);
			break;
		case CISCO_MODTYPE_ALLOW_VLAN: ;
			if(numbers[1] > 4096)
				return CISCO_ERROR_OUT_OF_RANGE;

			if(interface->allowedVlans->size > 2){
				void* tempVar = plGCRealloc(gc, interface->allowedVlans->array, (interface->allowedVlans->size + 1) * sizeof(uint16_t));

				if(!tempVar)
					return CISCO_ERROR_PL32LIB_GC;

				interface->allowedVlans->array = tempVar;
			}

			uint16_t* array = interface->allowedVlans->array;

			array[interface->allowedVlans->size] = numbers[1];
			interface->allowedVlans->size++;
			break;
		case CISCO_MODTYPE_IP_ADDR: ;
		case CISCO_MODTYPE_GATEWAY: ;
			char* testString;
			char* writeString;

			if(modType == CISCO_MODTYPE_GATEWAY){
				testString = interface->ipAddr;
				writeString = interface->gateway;
			}else{
				testString = interface->gateway;
				writeString = interface->ipAddr;
			}

			char* isIPv6[2] = { strchr(string, ':'), strchr(testString, ':') };
			size_t strSize = strlen(string);

			if(strcmp(testString, "") != 0) && ((isIPv6[0] && !isIPv6[1]) || (isIPv6[1] && !isIPv6[0])))
				return CISCO_ERROR_MISMATCHED_IPVER;

			if((!isIPv6[0] && strSize > 15) || (isIPv6[1] && strSize > 44))
				return CISCO_ERROR_BUFFER_OVERFLOW;

			memcpy(writeString, string, 44);
			break;
		case CISCO_MODTYPE_SUBMASK: ;
			if(number[1] > 32)
				return CISCO_ERROR_OUT_OF_RANGE;

			interface->subMask = number[1];
			break;
	}

	return 0;
}

// Adds an interface to a table
int ciscoAddInterface(ciscotable_t* table, ciscoint_t* interface){
	if(table->size > 1){
		void* tempPtr = plGCRealloc(gc, table->interfaces, (table->size + 1) * sizeof(ciscoint_t*));

		if(!tempPtr){
			return CISCO_ERROR_PL32LIB_GC;
		}

		table->interfaces = tempPtr;
	}

	table->interfaces[table->size] = interface;

	if(table->type == CISCO_TYPE_PORTCH)
		interface->mode = CISCO_MODE_IN_PORTCH;

	table->size++;

	return 0;
}

// Gets a pointer to an interface from a table
ciscoint_t* ciscoGetInterface(ciscotable_t* table, int index){
	if(index < 0 || index > table->size-1){
		return NULL;
	}

	return table->interfaces[index];
}

char* ciscoParseInterface(ciscoint_t* interface, plgc_t* gc){
	char* returnString = plGCMalloc(gc, 2 * sizeof(char));

	switch(interface->type){
		case CISCO_INT_F0:
			
	}
}

char* ciscoParseTable(ciscotable_t* table){
	//TODO: add code to parse tables
}

void ciscoPrintInterface(ciscoint_t* interface){
	//TODO: add code to print interfaces
}

void ciscoPrintTable(ciscotable_t* table){
	//TODO: add code to print tables
}
