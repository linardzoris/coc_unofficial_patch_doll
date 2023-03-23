#ifndef	common_functions_h_included
#define	common_functions_h_included

float4 convert_to_screen_space(float4 proj)
{
	float4 screen;
	screen.x = (proj.x + proj.w)*0.5;
	screen.y = (proj.w - proj.y)*0.5;
	screen.z = proj.z;
	screen.w = proj.w;
	return screen;
}

#endif	//	common_functions_h_included