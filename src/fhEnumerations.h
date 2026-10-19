#pragma once


// uint32_t
enum FH_ENTITY_TYPE {
						FH_ENTITY_TYPE_RENDER_OPAQUE = 0x00000000,		// 0-й бит
						FH_ENTITY_TYPE_RENDER_TRANSPARENT = 0x00000001,	// 0-1 бит
						FH_ENTITY_TYPE_RENDER_BLOOM = 0x00000002,		// 1-й
						FH_ENTITY_TYPE_LIGHT_VOLUME = 0x00000004,		// 2-1
						FH_ENTITY_TYPE_SKELETAL_MESH = 0x00000008		// 3-й
					};

// геометрический проход записывает в буферы информацию о цвете, нормали, позиции и т.д. для каждого пикселя
enum G_PASS_ATTACHMENTS {
							G_PASS_COLOR_ATTACHMENT = 0,
							G_PASS_NORMAL_ATTACHMENT = 1,
							G_PASS_POSITION_ATTACHMENT = 2,
							G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT = 3,
							G_PASS_EMISSION_ATTACHMENT = 4,
							G_PASS_DEPTH_ATTACHMENT = 5
						};


// проход освещения записывает в буферы информацию о финальном цвете фрагмента, а также итоговый кадр яркости (bloom)
enum L_PASS_ATTACHMENTS	{
							L_PASS_COLOR_ATTACHMENT = 0,
							L_PASS_BLOOM_ATTACHMENT = 1
						};


enum FH_DYNAMIC_LIGHTSOURCE_TYPE {
									FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL = 0,
									FH_DYNAMIC_LIGHTSOURCE_SPOT = 1,
									FH_DYNAMIC_LIGHTSOURCE_SUN = 2
								};


enum MAIN_MOVING_PARTS_TYPE { 	
								MMP_ROTATABLE, 							// MMP_ROTATABLE = 0 - вращаемый объект,
								MMP_ROTATABLE_REVERSE, 					// MMP_ROTATABLE_REVERSE = 1 - вращаемый туда и обратно,
								MMP_MOVABLE, 							// MMP_MOVABLE = 1 - сдвигаемый,
								MMP_MOVABLE_REVERSE, 					// MMP_MOVABLE_REVERSE - сдвигаемый туда и обратно
								MMP_ROTATABLE_MOVABLE, 
								MMP_ROTATABLE_MOVABLE_REVERSE, 
								MMP_ACTIVATE, 
								MMP_ACTIVATE_REVERSE 
							}; 
	    	
	
