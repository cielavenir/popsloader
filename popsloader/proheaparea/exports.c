#include <pspmoduleexport.h>
#define NULL ((void *) 0)

extern void module_start;
extern void module_stop;
extern void module_info;
static const unsigned int __syslib_exports[6] __attribute__((section(".rodata.sceResident"))) = {
	0xD632ACDB,
	0xCEE8593C,
	0xF01D73A7,
	(unsigned int) &module_start,
	(unsigned int) &module_stop,
	(unsigned int) &module_info,
};

extern void scePafHeaparea_F50AAE41;
extern void scePafHeaparea_ACCE25B2;
static const unsigned int __scePafHeaparea_exports[4] __attribute__((section(".rodata.sceResident"))) = {
	0xF50AAE41,
	0xACCE25B2,
	(unsigned int) &scePafHeaparea_F50AAE41,
	(unsigned int) &scePafHeaparea_ACCE25B2,
};

const struct _PspLibraryEntry __library_exports[2] __attribute__((section(".lib.ent"), used)) = {
	{ NULL, 0x0000, 0x8000, 4, 1, 2, &__syslib_exports },
	{ "scePafHeaparea", 0x0011, 0x0001, 4, 0, 2, &__scePafHeaparea_exports },
};
