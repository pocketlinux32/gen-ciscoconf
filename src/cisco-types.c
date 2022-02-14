/***********************************\
* Cisco Config Generator, v0.56     *
* (c)2021 pocketlinux32, Under GPL3 *
* Types Source File                 *
\***********************************/
#include <cisco/cisco-constants.h>
#include <cisco/cisco-types.h>

// Cisco Interface Structure
struct ciscoint {
	ciscoconst_t type;
	ciscoconst_t mode;
	int ports[2];
	char* name;
	int* allowed_vlans;
	char* ip_addr;
	int sub_mask;
	char* gateway;
}

// Cisco Table Structure
struct ciscotable {
	ciscoconst_t type;
	ciscoconst_t mode;
	int size;
	ciscoint_t** interfaces;
}

ciscoint_t* ciscoCreateInterface(ciscoconst_t type, int port1, int port2){
	ciscoint_t* returnInt = plGCMalloc(sizeof(ciscoint_t));

	returnInt->type = type;
	returnInt->mode = CISCO_MODE_ACCESS;
	returnInt->ports[0] = port1;
	returnInt->ports[1] = port2;
	returnInt->name = NULL;
	returnInt->allowed_vlans = NULL;
	returnInt->ip_addr = NULL;
	returnInt->sub_mask = 0;
	returnInt->gateway = NULL;

	return returnInt;
}

ciscotable_t* ciscoCreateTable(ciscoconst_t type, ciscoconst_t mode){
	ciscotable_t* returnTable = plGCMalloc(sizeof(ciscotable_t));

	returnTable->type = type;
	returnTable->mode = mode;
	returnTable->size = 0;
	returnTable->interfaces = plGCMalloc(2 * sizeof(ciscoint_t*));

	return returnTable;
}

void ciscoModifyInterface(ciscoint_t* interface, int mode, ...){
	//TODO: code to modify an interface
}

int ciscoAddInterface(ciscotable_t* table, ciscoint_t* interface){
	if(table->size > 1){
		void* tempPtr = plGCRealloc(table->interfaces, table->size * sizeof(ciscoint_t*));

		if(!tempPtr){
			return 1;
		}

		table->interfaces = tempPtr;
	}

	table->interfaces[table->size] = interface;
	interface->mode = CISCO_MODE_IN_VLAN;
	table->size++;

	return 0;
}

ciscoint_t* ciscoGetInterface(ciscotable_t* table, int index){
	if(index < 0 || index > table->size-1){
		return NULL;
	}

	return table->interfaces[index];
}

char* ciscoParseInterface(ciscoint_t* interface){
	//TODO: add code to parse interfaces
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
