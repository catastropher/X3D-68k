#include "X3D_platform.h"
#include "X3D_config.h"

#include "../../static/headers/X3D_enginestate.h"

void x3d_statusbar_add(const char* s) {
	printf("%s", s);
}

void x3d_load_interface(void) {
	printf("Load!");
	ngetchx();
	x3d->status.add = x3d_statusbar_add;
}

