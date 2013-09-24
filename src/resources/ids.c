#include "resources/ids.h"
#include <stdlib.h>

void get_filename_by_id(int id, char *ptr) {
	const char* path = "resources/";
	switch(id) {
        case SCENE_INTRO:    sprintf(ptr, "%sINTRO.BK", path);    break;
        case SCENE_MENU:     sprintf(ptr, "%sMAIN.BK", path);     break;
        case SCENE_ARENA0:   sprintf(ptr, "%sARENA0.BK", path);   break;
        case SCENE_ARENA1:   sprintf(ptr, "%sARENA1.BK", path);   break;
        case SCENE_ARENA2:   sprintf(ptr, "%sARENA2.BK", path);   break;
        case SCENE_ARENA3:   sprintf(ptr, "%sARENA3.BK", path);   break;
        case SCENE_ARENA4:   sprintf(ptr, "%sARENA4.BK", path);   break;
        case SCENE_ARENA5:   sprintf(ptr, "%sARENA5.BK", path);   break;
        case SCENE_NEWSROOM: sprintf(ptr, "%sNEWSROOM.BK", path); break;
        case SCENE_END:      sprintf(ptr, "%sEND.BK", path);      break;
        case SCENE_END1:     sprintf(ptr, "%sEND1.BK", path);     break;
        case SCENE_END2:     sprintf(ptr, "%sEND2.BK", path);     break;
        case SCENE_CREDITS:  sprintf(ptr, "%sCREDITS.BK", path);  break;
        case SCENE_MECHLAB:  sprintf(ptr, "%sMECHLAB.BK", path);  break;
        case SCENE_MELEE:    sprintf(ptr, "%sMELEE.BK", path);    break;
        case SCENE_VS:       sprintf(ptr, "%sVS.BK", path);       break;
        case SCENE_NORTHAM:  sprintf(ptr, "%sNORTH_AM.BK", path); break;
        case SCENE_KATUSHAI: sprintf(ptr, "%sKATUSHAI.BK", path); break;
        case SCENE_WAR:      sprintf(ptr, "%sWAR.BK", path);      break;
        case SCENE_WORLD:    sprintf(ptr, "%sWORLD.BK", path);    break;
        default:
        	ptr[0] = 0;
        	break;
	}
}