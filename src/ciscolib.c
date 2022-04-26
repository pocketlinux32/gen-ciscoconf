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
	char ipAddr[46];
	uint16_t subMask;
	char gateway[44];
}

// Cisco Table Structure
struct ciscotable {
	ciscoconst_t type;
	ciscoconst_t tableMode;
	ciscoconst_t intMode;
	char name[128];
	uint16_t* 
	plarray_t* interfaces;
}

plarray_t* ciscoCidrToOctet(uint8_t cidrMask, plgc_t* gc){
	if(cidrMask > 32)
		return NULL;

	uint8_t ipBytes[4] = { 0, 0, 0, 0 };
	uint8_t ipAdding[4] = { 128, 128, 128, 128 };

	for(int i = 0; i < cidrMask; i++){
		if(i < 8){
			ipBytes[0] += ipAdding[0];
			ipAdding[0] = ipAdding[0] >> 1;
		}else if(i < 16){
			ipBytes[1] += ipAdding[1];
			ipAdding[1] = ipAdding[1] >> 1;
		}else if(i < 24){
			ipBytes[2] += ipAdding[2];
			ipAdding[2] = ipAdding[2] >> 1;
		}else{
			ipBytes[3] += ipAdding[3];
			ipAdding[3] = ipAdding[3] >> 1;
		}
	}

	plarray_t* retArray = plGCAlloc(gc, sizeof(plarray_t));
	retArray->array = plGCAlloc(gc, 4 * sizeof(uint8_t));
	retArray->size = 4;
	memcpy(retArray->array, ipAdding, 4 * sizeof(uint8_t));

	return retArray;
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
ciscotable_t* ciscoCreateTable(ciscoconst_t type, ciscoconst_t tmode, ciscoconst_t imode, plgc_t* gc){
	ciscotable_t* returnTable = plGCAlloc(gc, sizeof(ciscotable_t));

	returnTable->type = type;
	returnTable->tableMode = tmode;
	returnTable->intMode = imode;
	returnTable->interfaces = plGCAlloc(gc, sizeof(plarray_t));
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

			if((!isIPv6[0] && strSize > 15) || (isIPv6[1] && strSize > 45))
				return CISCO_ERROR_BUFFER_OVERFLOW;

			memcpy(writeString, string, strSize);
			writeString
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
int ciscoAddInterface(ciscotable_t* table, ciscoint_t* interface, plgc_t* gc){
	if(table->size > 1){
		void* tempPtr = plGCRealloc(gc, table->interfaces->array, (table->interfaces->size + 1) * sizeof(ciscoint_t*));

		if(!tempPtr){
			return CISCO_ERROR_PL32LIB_GC;
		}

		table->interfaces->array = tempPtr;
	}

	ciscoint_t** array = table->interfaces->array
	array[table->interfaces->size] = interface;

	if(table->type == CISCO_TYPE_PORTCH)
		interface->mode = CISCO_MODE_IN_PORTCH;

	table->interfaces->size++;

	return 0;
}

// Gets a pointer to an interface from a table
ciscoint_t* ciscoGetInterface(ciscotable_t* table, int index){
	if(index < 0 || index > table->size-1){
		return NULL;
	}

	return ((ciscoint_t**)table->interfaces->array)[index];
}

plfile_t* ciscoParseInterface(ciscoint_t* interface, plgc_t* gc){
	plfile_t* returnBuffer = plFOpen(NULL, "w+", gc);
	char* placeholder = plGCAlloc(gc, 16 * sizeof(char*));
	char cmdline[8192] = "";

	switch(interface->type){
		case CISCO_INT_F0: ;
			strcpy(placeholder, "f0");
			break;
		case CISCO_INT_G0: ;
			strcpy(placeholder, "g0");
			break;
		case CISCO_INT_G00: ;
			strcpy(placeholder, "g0/0");
			break;
		case CISCO_INT_G01: ;
			strcpy(placeholder, "g0/1");
			break;
		case CISCO_INT_S00: ;
			strcpy(placeholder, "s0/0");
			break;
		case CISCO_INT_S01: ;
			strcpy(placeholder, "s0/1");
			break;
	}

	if(interface->number[0] == interface->number[1]){
		sprintf(cmdline, "int %s/%d\n", placeholder, interface->number[0])
	}else{
		sprintf(cmdline, "int range %s/%d-%d\n", placeholder, interface->number[0], interface[1]);
	}

	plPuts(returnBuffer, cmdline);
	for(int i = 0; i < 16; i++)
		placeholder[i] = 0;

	for(int i = 0; i < 8192; i++)
		cmdline[i] = 0;

	switch(interface->mode){
		case CISCO_MODE_ACCESS: ;
			plPuts(returnBuffer, "switchport mode access\n");
			break;
		case CISCO_MODE_TRUNK: ;
			plPuts(returnBuffer, "switchport mode trunk\n");
			break;
		case CISCO_MODE_AUTO: ;
			plPuts(returnBuffer, "switchport mode auto\n");
			break;
	}

	if(strcmp(interface->ipAddr, "") != 0){
		char* isIpAddrV6 = strchr(interface->ipAddr, ':');

		if(!isIpAddrV6){
			plarray_t* octetSubmask = ciscoCidrToOctet(interface->subMask, gc);
			uint8_t* array = octetSubmask->array;
			sprintf(cmdline, "ip address %s %d.%d.%d.%d\n", interface->ipAddr, array[0], array[1], array[2], array[3]);
			plShellFreeArray(octetSubmask, false, gc);
		}else{
			sprintf(cmdline, "ipv6 address %s/%d\n", interface->ipAddr, interface->subMask);
		}

		plPuts(returnBuffer, cmdline);
		for(int i = 0; i < 8192; i++)
			cmdline[i] = 0;
	}

	if(strcmp(interface->gateway, "") != 0){
		char* isIpAddrV6 = strchr(interface->ipAddr, ':');

		if(!isIpAddrV6)
			plPuts(returnBuffer, "ip default-gateway %s\n", interface->gateway);

		plPuts(returnBuffer, cmdline);
		for(int i = 0; i < 8192; i++)
			cmdline[i] = 0;
	}

	plGCFree(gc, placeholder);
	plPuts(returnBuffer, "exit\n");
	return returnBuffer;
}

plfile_t* ciscoParseTable(ciscotable_t* table, plgc_t* gc){
	plfile_t* returnBuffer = plOpen(NULL, "w+", gc);
	ciscoint_t** array = table->interfaces->array;

	for(int i = 0; i < table->interfaces; i++){
		plfile_t* tempFile = ciscoParseInterface(array[i], gc);
		switch(table->type){
			case CISCO_INT_VLAN:
				
		}

		plFCat(returnBuffer, tempFile, true);
	}


	if(table->type == CISCO_INT_PORTCH){
		char*
		switch(table->mode){
			case CISCO_MODE_ACTIVE: ;
				strcpy(placeholder, "access");
				break;
			case CISCO_MODE_PASSIVE: ;
				strcpy(placeholder, "access");
				break;
			case CISCO_MODE_DESIRABLE: ;
				strcpy(placeholder, "access");
				break;
			case CISCO_MODE_AUTO: ;
				strcpy(placeholder, "access");
				break;
		}
	}
}

void ciscoPrintInterface(ciscoint_t* interface){
	//TODO: add code to print interfaces
}

void ciscoPrintTable(ciscotable_t* table){
	//TODO: add code to print tables
}
