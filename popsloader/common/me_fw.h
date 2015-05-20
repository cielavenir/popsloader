// This is a part of CFW ME.
//
// popsbridge v3 source code by neur0ner
// merged to popsloader by popsdeco
//
// this file must be included only once. (perhaps to main.c)

//#include "systemctrl.h"
void ClearCaches()
{
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();	
}

void* search_module_export(SceModule2 *pMod, const char *szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	void *entTab = pMod->ent_top;
	int entLen = pMod->ent_size;
	int i = 0;

	while(i < entLen) {
		int count;
		int total;
		unsigned int *vars;
		entry = (struct SceLibraryEntryTable *) (entTab + i);

		if( (entry->libname == szLib) || (entry->libname && strcmp(entry->libname, szLib) == 0)) {
			total = entry->stubcount + entry->vstubcount;		
			vars = entry->entrytable;

			if(total > 0) {
				for(count = 0; count < total ; count++) {
					if (vars[count] == nid) 
					{
						return (void *)(vars[count+total]);
					}
				}
			}
		}

		i += (entry->len * 4);
	}
	return NULL;
}

void* search_module_stub(SceModule2 *pMod, const char *szLib, u32 nid)
{
	void *entTab = pMod ->stub_top;
	int entLen = pMod->stub_size;
	struct SceLibraryStubTable *current;
	int i = 0 ,j;

	while( i < entLen ) {
		current = (struct SceLibraryStubTable *)(entTab + i);
		if(strcmp(current->libname, szLib ) == 0) {
			for(j=0;j< current->stubcount ;j++) {
				if( current->nidtable[j] == nid ) {
					return (void *)((u32)(current->stubtable) + 8*j );
				}
			}

			break;
		}
		i += (current->len * 4);
	}

	return NULL;
}


int (*StartModuleEx)(int , SceSize , void *, int *, SceKernelSMOption *);
int (*pro_func)(int , SceSize , void *, int *, SceKernelSMOption *);

int hook_start_module(int modid, SceSize argsize, void *argp, int *modstatus, SceKernelSMOption *opt)
{
	int ret;
	SceModule2 *mod = (SceModule2*) sceKernelFindModuleByUID(modid);			

	if( strcmp(mod->modname, "sceMediaSync") == 0 ) {
		sceKernelStartModule(modid, argsize, argp, modstatus, opt);

		int dummy = sceKernelLoadModule("flash0:/kd/idmanager.prx", 0, NULL);
		mod = (SceModule2*) sceKernelFindModuleByUID(dummy);
		strcpy( (char *)mod->modname , "PROPopcornManager");
		modid = dummy;
		ClearCaches();
	}

	ret = pro_func( modid,  argsize, argp, modstatus, opt);
//	printf(" ret = 0x%08X \n", ret);
	
	if(ret >= 0 ) {
		return ret;
	}

	if( StartModuleEx ) {
		return StartModuleEx( modid,  argsize, argp, modstatus, opt);
	}

	return -1;
}

#if 1
//ndef POPCORN
//void *sctrlSetStartModuleExtra( int (* func)());
void* (*sctrlSetStartModuleExtra_k)( int (* func)());

void sctrlSetCustomStartModule_hook(int (*func)(int modid, SceSize argsize, void *argp, int *modstatus, SceKernelSMOption *opt))
{
	pro_func = func;
	StartModuleEx = sctrlSetStartModuleExtra_k( hook_start_module );
}
#endif

//patch for popsloader v3
#if 0
int strcmp_patch(const char *s1, const char *s2)
{
	int ret = strcmp(s1,s2);
	if( ret == 0 &&  strcmp( s2, "pops") == 0) {
		if( sceKernelFindModuleByName("popscore") != NULL)
			ret = -1;
	}

	return ret;
}
#endif

u32 sceKernelQuerySystemCall(void *a0);
u32 sctrlKernelQuerySystemCall_patch(void *addr)
{
	u32 k1 = pspSdkSetK1(0);
	u32 ret = sceKernelQuerySystemCall(addr);
	pspSdkSetK1(k1);
	return ret;
}

//patch for popsloader v1
int sctrlKernelLoadExecVSHWithApitype_patch(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param)
{
	while( sceKernelFindModuleByName("sceKernelLibrary") == NULL) {
		sceKernelDelayThread(100*1000);
	}

	int ret = 0;
	if( sceKernelFindModuleByName("Idchange_Driver") != NULL ) {
		ret = sctrlKernelLoadExecVSHWithApitype( apitype, file, param );
	}

	return ret;
}

int sceKernelApplyPspRelSectionPatched(SceModule2 *mod)
{
//	u32 text_addr = mod->text_addr;
	char *modinfo=mod->modname;
	void *addr;

	if (strcmp(modinfo, "Idchange_Driver") == 0 && sceKernelFindModuleByName("popscore") != NULL ) {

		//module_start
		//addr = search_module_export( mod , NULL , 0xD632ACDB );
		addr = (void *)mod->entry_addr;
		MAKE_DUMMY_FUNCTION_RETURN_1((u32)addr);
		ClearCaches();
	}
#if 0 //popscore.prx resolves me_fw on its own.
	else if( strcmp(modinfo, "popscore") == 0 ) {

		//sctrlSetCustomStartModule
		addr = search_module_stub( mod , "SystemCtrlForKernel" , 0x259B51CE );
		REDIRECT_FUNCTION((u32)addr, sctrlSetCustomStartModule_hook );

		//sctrlKernelQuerySystemCall
		addr = search_module_stub( mod , "SystemCtrlForKernel" , 0x56CEAF00 );

		if( addr ) {
			REDIRECT_FUNCTION((u32)addr, sctrlKernelQuerySystemCall_patch );
		}

		ClearCaches();
	}
#endif
#if 0 //popsloader.prx resolves me_fw on its own.
	else if( strcmp(modinfo, "popsloader") == 0 ) {

		//strcmp
		addr = search_module_stub( mod , "SysclibForKernel" , 0xC0AB8932 );
		REDIRECT_FUNCTION((u32)addr, strcmp_patch );

		//sctrlKernelLoadExecVSHWithApitype
		addr = search_module_stub( mod , "SystemCtrlForKernel" , 0x2D10FB28 );
		REDIRECT_FUNCTION((u32)addr, sctrlKernelLoadExecVSHWithApitype_patch );
		ClearCaches();
	}
#endif
#if 0 //popcorn.prx resolves me_fw on its own.
	else if( strcmp( modinfo, "PROPopcornManager") == 0) {

		//sctrlKernelQuerySystemCall
		addr = search_module_stub( mod , "SystemCtrlForKernel" , 0x56CEAF00 );
		if( addr ) {
			REDIRECT_FUNCTION((u32)addr, sctrlKernelQuerySystemCall_patch );
			ClearCaches();
		}
	}
#endif
#if 0
	if( g_previous ) {  
		return g_previous( mod );       
	}
#endif
	return 0;
}

u32 me_fw;
void SetME(){ //must be called in module_start, only once
	//me_fw = 0;

	//codestation's method
	//me_fw = sctrlHENFindFunction("SystemControl", "VersionSpoofer", 0x5B18622C)==0;

	//neur0ner's method
	sctrlSetStartModuleExtra_k=(void*)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0x221400A6);
	me_fw = sctrlSetStartModuleExtra_k!=0;

	//Anyway I don't know anything about TN's pops.
}
