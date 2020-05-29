
//----------------------------------------------------------------------------
// INCLUDED HEADER FILES
//----------------------------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS

extern "C"
{

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

#define TRUE 1
#define FALSE 0

//----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//----------------------------------------------------------------------------


void __cdecl initialize(void);
void __cdecl read_boxlist_input(void);
void __cdecl execute_iterations(void); //TODO: Needs a better name yet
void __cdecl list_candidate_layers(void);
int __cdecl compute_layer_list(const void* i, const void* j);
int __cdecl pack_layer(void);
int __cdecl find_layer(short int thickness);
void __cdecl find_box(short int hmx, short int hy, short int hmy, short int hz, short int hmz);
void __cdecl analyze_box(short int hmx, short int hy, short int hmy, short int hz, short int hmz, short int dim1, short int dim2, short int dim3);
void __cdecl find_smallest_z(void);
void __cdecl checkfound(void); //TODO: Find better name for this
void __cdecl volume_check(void);
void __cdecl write_visualization_data_file(void);
void __cdecl write_boxlist_file(void);
void __cdecl report_results(void);

//----------------------------------------------------------------------------
// VARIABLE, CONSTANT AND STRUCTURE DECLARATIONS
//----------------------------------------------------------------------------

char strpx[5], strpy[5], strpz[5];
char strcox[5], strcoy[5], strcoz[5];
char strpackx[5], strpacky[5], strpackz[5];

char* filename = NULL;
char packing;
char layerdone;
char evened;
char variant;
char best_variant;
char packingbest;
char hundredpercent;
char graphout[] = ".~3d";
char unpacked;

short int boxx, boxy, boxz, boxi;
short int bboxx, bboxy, bboxz, bboxi;
short int cboxx, cboxy, cboxz, cboxi;
short int bfx, bfy, bfz;
short int bbfx, bbfy, bbfz;
short int xx, yy, zz;
short int pallet_x, pallet_y, pallet_z;

short int total_boxes;
short int x;
short int n;
short int layerlistlen;
short int layerinlayer;
short int prelayer;
short int lilz;
short int number_of_iterations;
short int hour;
short int min;
short int sec;
short int layersindex;
short int remainpx, remainpy, remainpz;
short int packedy;
short int prepackedy;
short int layerthickness;
short int itelayer;
short int preremainpy;
short int best_iteration;
short int packednumbox;
short int number_packed_boxes;

double packedvolume;
double best_solution_volume;
double total_pallet_volume;
double total_box_volume;
double temp;
double pallet_volume_used_percentage;
double packed_box_percentage;
double elapsed_time;

struct boxinfo {
	char is_packed;
	short int dim1, dim2, dim3, n, cox, coy, coz, packx, packy, packz;
	long int vol;
} boxlist[5000];

struct layerlist {
	long int layereval;
	short int layerdim;
} layers[1000];

struct scrappad {
	struct scrappad* prev, * next;
	short int cumx, cumz;
};

struct scrappad* scrapfirst, * scrapmemb, * smallestz, * trash;

time_t start, finish;

FILE* boxlist_input_file, * report_output_file, * visualizer_file;

char version[] = "0.02 dynamic lib version";

//----------------------------------------------------------------------------
// MAIN PROGRAM
//----------------------------------------------------------------------------

int run(char* param_filename)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	//Parse Command line options
	memcpy ( &filename, &param_filename, sizeof(param_filename) );
	//filename = param_filename;


	initialize();
	time(&start);
	execute_iterations();
	time(&finish);
	report_results();
	return(0);
}

//----------------------------------------------------------------------------
// PERFORMS INITIALIZATIONS
//----------------------------------------------------------------------------

void initialize(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	read_boxlist_input();
	temp = 1.0;
	total_pallet_volume = temp * xx * yy * zz;
	total_box_volume = 0.0;
	for (x = 1; x <= total_boxes; x++) {
		total_box_volume = total_box_volume + boxlist[x].vol;
	}

	scrapfirst = (scrappad*)malloc(sizeof(struct scrappad));
	if (scrapfirst == NULL)
	{
		printf("Insufficient memory available\n");
		exit(1);
	}
	scrapfirst->prev = NULL;
	scrapfirst->next = NULL;
	best_solution_volume = 0.0;
	packingbest = 0;
	hundredpercent = 0;
	number_of_iterations = 0;
}

//----------------------------------------------------------------------------
// READS THE PALLET AND BOX SET DATA ENTERED BY THE USER FROM THE INPUT FILE
//----------------------------------------------------------------------------

void read_boxlist_input(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	short int n;
	//TODO: Robustify this so the box label can be larger and have spaces
	char lbl[5], dim1[5], dim2[5], dim3[5], boxn[5], strxx[5], stryy[5], strzz[5];

	if ((boxlist_input_file = fopen(filename, "r")) == NULL)
	{
		printf("Cannot open file %s\n", filename);
		exit(1);
	}
	total_boxes = 1;

	if (fscanf(boxlist_input_file, "%s %s %s", strxx, stryy, strzz) == EOF)
	{
		exit(1);
	}

	xx = atoi(strxx);
	yy = atoi(stryy);
	zz = atoi(strzz);

	while (fscanf(boxlist_input_file, "%s %s %s %s %s", lbl, dim1, dim2, dim3, boxn) != EOF)
	{
		boxlist[total_boxes].dim1 = atoi(dim1);
		boxlist[total_boxes].dim2 = atoi(dim2);
		boxlist[total_boxes].dim3 = atoi(dim3);

		boxlist[total_boxes].vol = boxlist[total_boxes].dim1 * boxlist[total_boxes].dim2 * boxlist[total_boxes].dim3;
		n = atoi(boxn);
		boxlist[total_boxes].n = n;

		while (--n)
		{
			boxlist[total_boxes + n] = boxlist[total_boxes];
		}
		total_boxes = total_boxes + atoi(boxn);
	}
	--total_boxes;
	fclose(boxlist_input_file);
	return;
}

//----------------------------------------------------------------------------
// ITERATIONS ARE DONE AND PARAMETERS OF THE BEST SOLUTION ARE FOUND
//----------------------------------------------------------------------------

void execute_iterations(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	for (variant = 1; variant <= 6; variant++)
	{
		switch (variant)
		{
		case 1:
			pallet_x = xx; pallet_y = yy; pallet_z = zz;
			break;
		case 2:
			pallet_x = zz; pallet_y = yy; pallet_z = xx;
			break;
		case 3:
			pallet_x = zz; pallet_y = xx; pallet_z = yy;
			break;
		case 4:
			pallet_x = yy; pallet_y = xx; pallet_z = zz;
			break;
		case 5:
			pallet_x = xx; pallet_y = zz; pallet_z = yy;
			break;
		case 6:
			pallet_x = yy; pallet_y = zz; pallet_z = xx;
			break;
		}

		list_candidate_layers();
		layers[0].layereval = -1;
		qsort(layers, layerlistlen + 1, sizeof(struct layerlist), compute_layer_list);

		for (layersindex = 1; layersindex <= layerlistlen; layersindex++)
		{
			++number_of_iterations;
			time(&finish);
			elapsed_time = difftime(finish, start);
			printf("VARIANT: %5d; ITERATION (TOTAL): %5d; BEST SO FAR: %.3f %%; TIME: %.0f", variant, number_of_iterations, pallet_volume_used_percentage, elapsed_time);
			packedvolume = 0.0;
			packedy = 0;
			packing = 1;
			layerthickness = layers[layersindex].layerdim;
			itelayer = layersindex;
			remainpy = pallet_y;
			remainpz = pallet_z;
			packednumbox = 0;
			for (x = 1; x <= total_boxes; x++)
			{
				boxlist[x].is_packed = FALSE;
			}

			//BEGIN DO-WHILE
			do
			{
				layerinlayer = 0;
				layerdone = 0;
				if (pack_layer())
				{
					exit(1);
				}
				packedy = packedy + layerthickness;
				remainpy = pallet_y - packedy;
				if (layerinlayer)
				{
					prepackedy = packedy;
					preremainpy = remainpy;
					remainpy = layerthickness - prelayer;
					packedy = packedy - layerthickness + prelayer;
					remainpz = lilz;
					layerthickness = layerinlayer;
					layerdone = 0;
					if (pack_layer())
					{
						exit(1);
					}
					packedy = prepackedy;
					remainpy = preremainpy;
					remainpz = pallet_z;
				}
				find_layer(remainpy);
			} while (packing);
			// END DO-WHILE

			if (packedvolume > best_solution_volume)
			{
				best_solution_volume = packedvolume;
				best_variant = variant;
				best_iteration = itelayer;
				number_packed_boxes = packednumbox;
			}

			if (hundredpercent) break;
			pallet_volume_used_percentage = best_solution_volume * 100 / total_pallet_volume;
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		}
		if (hundredpercent) break;
		if ((xx == yy) && (yy == zz)) variant = 6;
	}
}

//----------------------------------------------------------------------------
// LISTS ALL POSSIBLE LAYER HEIGHTS BY GIVING A WEIGHT VALUE TO EACH OF THEM.
//----------------------------------------------------------------------------

void list_candidate_layers(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	char same;
	short int exdim, dimdif, dimen2, dimen3, y, z, k;
	long int layereval;

	layerlistlen = 0;

	for (x = 1; x <= total_boxes; x++)
	{
		for (y = 1; y <= 3; y++)
		{
			switch (y)
			{
			case 1:
				exdim = boxlist[x].dim1;
				dimen2 = boxlist[x].dim2;
				dimen3 = boxlist[x].dim3;
				break;
			case 2:
				exdim = boxlist[x].dim2;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim3;
				break;
			case 3:
				exdim = boxlist[x].dim3;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim2;
				break;
			}
			if ((exdim > pallet_y) || (((dimen2 > pallet_x) || (dimen3 > pallet_z)) && ((dimen3 > pallet_x) || (dimen2 > pallet_z)))) continue;
			same = 0;

			for (k = 1; k <= layerlistlen; k++)
			{
				if (exdim == layers[k].layerdim)
				{
					same = 1;
					continue;
				}
			}
			if (same) continue;
			layereval = 0;
			for (z = 1; z <= total_boxes; z++)
			{
				if (!(x == z))
				{
					dimdif = abs(exdim - boxlist[z].dim1);
					if (abs(exdim - boxlist[z].dim2) < dimdif)
					{
						dimdif = abs(exdim - boxlist[z].dim2);
					}
					if (abs(exdim - boxlist[z].dim3) < dimdif)
					{
						dimdif = abs(exdim - boxlist[z].dim3);
					}
					layereval = layereval + dimdif;
				}
			}
			layers[++layerlistlen].layereval = layereval;
			layers[layerlistlen].layerdim = exdim;
		}
	}
	return;
}

//----------------------------------------------------------------------------
// REQUIRED FUNCTION FOR QSORT FUNCTION TO WORK
//----------------------------------------------------------------------------

int compute_layer_list(const void* i, const void* j)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	return *(long int*)i - *(long int*)j;
}

//----------------------------------------------------------------------------
// PACKS THE BOXES FOUND AND ARRANGES ALL VARIABLES AND RECORDS PROPERLY
//----------------------------------------------------------------------------

int pack_layer(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	short int lenx, lenz, lpz;

	if (!layerthickness)
	{
		packing = 0;
		return 0;
	}

	scrapfirst->cumx = pallet_x;
	scrapfirst->cumz = 0;

	while (1)
	{
		find_smallest_z();

		if (!smallestz->prev && !smallestz->next)
		{
			//*** SITUATION-1: NO BOXES ON THE RIGHT AND LEFT SIDES ***

			lenx = smallestz->cumx;
			lpz = remainpz - smallestz->cumz;
			find_box(lenx, layerthickness, remainpy, lpz, lpz);
			checkfound();

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].cox = 0;
			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			if (cboxx == smallestz->cumx)
			{
				smallestz->cumz = smallestz->cumz + cboxz;
			}
			else
			{
				smallestz->next = (scrappad*)malloc(sizeof(struct scrappad));
				if (smallestz->next == NULL)
				{
					printf("Insufficient memory available\n");
					return 1;
				}
				smallestz->next->next = NULL;
				smallestz->next->prev = smallestz;
				smallestz->next->cumx = smallestz->cumx;
				smallestz->next->cumz = smallestz->cumz;
				smallestz->cumx = cboxx;
				smallestz->cumz = smallestz->cumz + cboxz;
			}
			volume_check();
		}
		else if (!smallestz->prev)
		{
			//*** SITUATION-2: NO BOXES ON THE LEFT SIDE ***

			lenx = smallestz->cumx;
			lenz = smallestz->next->cumz - smallestz->cumz;
			lpz = remainpz - smallestz->cumz;
			find_box(lenx, layerthickness, remainpy, lenz, lpz);
			checkfound();

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			if (cboxx == smallestz->cumx)
			{
				boxlist[cboxi].cox = 0;
				if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					smallestz->cumz = smallestz->next->cumz;
					smallestz->cumx = smallestz->next->cumx;
					trash = smallestz->next;
					smallestz->next = smallestz->next->next;
					if (smallestz->next)
					{
						smallestz->next->prev = smallestz;
					}
					free(trash);
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				boxlist[cboxi].cox = smallestz->cumx - cboxx;
				if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					smallestz->cumx = smallestz->cumx - cboxx;
				}
				else
				{
					smallestz->next->prev = (scrappad*)malloc(sizeof(struct scrappad));
					if (smallestz->next->prev == NULL)
					{
						printf("Insufficient memory available\n");
						return 1;
					}
					smallestz->next->prev->next = smallestz->next;
					smallestz->next->prev->prev = smallestz;
					smallestz->next = smallestz->next->prev;
					smallestz->next->cumx = smallestz->cumx;
					smallestz->cumx = smallestz->cumx - cboxx;
					smallestz->next->cumz = smallestz->cumz + cboxz;
				}
			}
			volume_check();
		}
		else if (!smallestz->next)
		{
			//*** SITUATION-3: NO BOXES ON THE RIGHT SIDE ***

			lenx = smallestz->cumx - smallestz->prev->cumx;
			lenz = smallestz->prev->cumz - smallestz->cumz;
			lpz = remainpz - (*smallestz).cumz;
			find_box(lenx, layerthickness, remainpy, lenz, lpz);
			checkfound();

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			boxlist[cboxi].cox = smallestz->prev->cumx;

			if (cboxx == smallestz->cumx - smallestz->prev->cumx)
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->cumx;
					smallestz->prev->next = NULL;
					free(smallestz);
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->prev->cumx + cboxx;
				}
				else
				{
					smallestz->prev->next = (scrappad*)malloc(sizeof(struct scrappad));
					if (smallestz->prev->next == NULL)
					{
						printf("Insufficient memory available\n");
						return 1;
					}
					smallestz->prev->next->prev = smallestz->prev;
					smallestz->prev->next->next = smallestz;
					smallestz->prev = smallestz->prev->next;
					smallestz->prev->cumx = smallestz->prev->prev->cumx + cboxx;
					smallestz->prev->cumz = smallestz->cumz + cboxz;
				}
			}
			volume_check();
		}
		else if (smallestz->prev->cumz == smallestz->next->cumz)
		{
			//*** SITUATION-4: THERE ARE BOXES ON BOTH OF THE SIDES ***

			//*** SUBSITUATION-4A: SIDES ARE EQUAL TO EACH OTHER ***

			lenx = smallestz->cumx - smallestz->prev->cumx;
			lenz = smallestz->prev->cumz - smallestz->cumz;
			lpz = remainpz - smallestz->cumz;

			find_box(lenx, layerthickness, remainpy, lenz, lpz);
			checkfound();

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			if (cboxx == smallestz->cumx - smallestz->prev->cumx)
			{
				boxlist[cboxi].cox = smallestz->prev->cumx;
				if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					smallestz->prev->cumx = smallestz->next->cumx;
					if (smallestz->next->next)
					{
						smallestz->prev->next = smallestz->next->next;
						smallestz->next->next->prev = smallestz->prev;
						free(smallestz);
					}
					else
					{
						smallestz->prev->next = NULL;
						free(smallestz);
					}
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else if (smallestz->prev->cumx < pallet_x - smallestz->cumx)
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->cumx = smallestz->cumx - cboxx;
					boxlist[cboxi].cox = smallestz->cumx - cboxx;
				}
				else
				{
					boxlist[cboxi].cox = smallestz->prev->cumx;
					smallestz->prev->next = (scrappad*)malloc(sizeof(struct scrappad));
					if (smallestz->prev->next == NULL)
					{
						printf("Insufficient memory available\n");
						return 1;
					}
					smallestz->prev->next->prev = smallestz->prev;
					smallestz->prev->next->next = smallestz;
					smallestz->prev = smallestz->prev->next;
					smallestz->prev->cumx = smallestz->prev->prev->cumx + cboxx;
					smallestz->prev->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->prev->cumx + cboxx;
					boxlist[cboxi].cox = smallestz->prev->cumx;
				}
				else
				{
					boxlist[cboxi].cox = smallestz->cumx - cboxx;
					smallestz->next->prev = (scrappad*)malloc(sizeof(struct scrappad));
					if (smallestz->next->prev == NULL)
					{
						printf("Insufficient memory available\n");
						return 1;
					}
					smallestz->next->prev->next = smallestz->next;
					smallestz->next->prev->prev = smallestz;
					smallestz->next = smallestz->next->prev;
					smallestz->next->cumx = smallestz->cumx;
					smallestz->next->cumz = smallestz->cumz + cboxz;
					smallestz->cumx = smallestz->cumx - cboxx;
				}
			}
			volume_check();
		}
		else
		{
			//*** SUBSITUATION-4B: SIDES ARE NOT EQUAL TO EACH OTHER ***

			lenx = smallestz->cumx - smallestz->prev->cumx;
			lenz = smallestz->prev->cumz - smallestz->cumz;
			lpz = remainpz - smallestz->cumz;
			find_box(lenx, layerthickness, remainpy, lenz, lpz);
			checkfound();

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			boxlist[cboxi].cox = smallestz->prev->cumx;
			if (cboxx == smallestz->cumx - smallestz->prev->cumx)
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->cumx;
					smallestz->prev->next = smallestz->next;
					smallestz->next->prev = smallestz->prev;
					free(smallestz);
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->prev->cumx + cboxx;
				}
				else if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					boxlist[cboxi].cox = smallestz->cumx - cboxx;
					smallestz->cumx = smallestz->cumx - cboxx;
				}
				else
				{
					smallestz->prev->next = (scrappad*)malloc(sizeof(struct scrappad));
					if (smallestz->prev->next == NULL)
					{
						printf("Insufficient memory available\n");
						return 1;
					}
					smallestz->prev->next->prev = smallestz->prev;
					smallestz->prev->next->next = smallestz;
					smallestz->prev = smallestz->prev->next;
					smallestz->prev->cumx = smallestz->prev->prev->cumx + cboxx;
					smallestz->prev->cumz = smallestz->cumz + cboxz;
				}
			}
			volume_check();
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// FINDS THE MOST PROPER LAYER HIGHT BY LOOKING AT THE UNPACKED BOXES AND THE
// REMAINING EMPTY SPACE AVAILABLE
//----------------------------------------------------------------------------

int find_layer(short int thickness)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	short int exdim, dimdif, dimen2, dimen3, y, z;
	long int layereval, eval;
	layerthickness = 0;
	eval = 1000000;
	for (x = 1; x <= total_boxes; x++)
	{
		if (boxlist[x].is_packed) continue;
		for (y = 1; y <= 3; y++)
		{
			switch (y)
			{
			case 1:
				exdim = boxlist[x].dim1;
				dimen2 = boxlist[x].dim2;
				dimen3 = boxlist[x].dim3;
				break;
			case 2:
				exdim = boxlist[x].dim2;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim3;
				break;
			case 3:
				exdim = boxlist[x].dim3;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim2;
				break;
			}
			layereval = 0;
			if ((exdim <= thickness) && (((dimen2 <= pallet_x) && (dimen3 <= pallet_z)) || ((dimen3 <= pallet_x) && (dimen2 <= pallet_z))))
			{
				for (z = 1; z <= total_boxes; z++)
				{
					if (!(x == z) && !(boxlist[z].is_packed))
					{
						dimdif = abs(exdim - boxlist[z].dim1);
						if (abs(exdim - boxlist[z].dim2) < dimdif)
						{
							dimdif = abs(exdim - boxlist[z].dim2);
						}
						if (abs(exdim - boxlist[z].dim3) < dimdif)
						{
							dimdif = abs(exdim - boxlist[z].dim3);
						}
						layereval = layereval + dimdif;
					}
				}
				if (layereval < eval)
				{
					eval = layereval;
					layerthickness = exdim;
				}
			}
		}
	}
	if (layerthickness == 0 || layerthickness > remainpy) packing = 0;
	return 0;
}

//----------------------------------------------------------------------------
// FINDS THE MOST PROPER BOXES BY LOOKING AT ALL SIX POSSIBLE ORIENTATIONS,
// EMPTY SPACE GIVEN, ADJACENT BOXES, AND PALLET LIMITS
//----------------------------------------------------------------------------

void find_box(short int hmx, short int hy, short int hmy, short int hz, short int hmz)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	short int y;
	bfx = 32767; bfy = 32767; bfz = 32767;
	bbfx = 32767; bbfy = 32767; bbfz = 32767;
	boxi = 0; bboxi = 0;
	for (y = 1; y <= total_boxes; y = y + boxlist[y].n)
	{
		for (x = y; x < x + boxlist[y].n - 1; x++)
		{
			if (!boxlist[x].is_packed) break;
		}
		if (boxlist[x].is_packed) continue;
		if (x > total_boxes) return;
		analyze_box(hmx, hy, hmy, hz, hmz, boxlist[x].dim1, boxlist[x].dim2, boxlist[x].dim3);
		if ((boxlist[x].dim1 == boxlist[x].dim3) && (boxlist[x].dim3 == boxlist[x].dim2)) continue;
		analyze_box(hmx, hy, hmy, hz, hmz, boxlist[x].dim1, boxlist[x].dim3, boxlist[x].dim2);
		analyze_box(hmx, hy, hmy, hz, hmz, boxlist[x].dim2, boxlist[x].dim1, boxlist[x].dim3);
		analyze_box(hmx, hy, hmy, hz, hmz, boxlist[x].dim2, boxlist[x].dim3, boxlist[x].dim1);
		analyze_box(hmx, hy, hmy, hz, hmz, boxlist[x].dim3, boxlist[x].dim1, boxlist[x].dim2);
		analyze_box(hmx, hy, hmy, hz, hmz, boxlist[x].dim3, boxlist[x].dim2, boxlist[x].dim1);
	}
}

//----------------------------------------------------------------------------
// ANALYZES EACH UNPACKED BOX TO FIND THE BEST FITTING ONE TO THE EMPTY SPACE
// GIVEN
//----------------------------------------------------------------------------

void analyze_box(short int hmx, short int hy, short int hmy, short int hz, short int hmz, short int dim1, short int dim2, short int dim3)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	if (dim1 <= hmx && dim2 <= hmy && dim3 <= hmz)
	{
		if (dim2 <= hy)
		{
			if (hy - dim2 < bfy)
			{
				boxx = dim1;
				boxy = dim2;
				boxz = dim3;
				bfx = hmx - dim1;
				bfy = hy - dim2;
				bfz = abs(hz - dim3);
				boxi = x;
			}
			else if (hy - dim2 == bfy && hmx - dim1 < bfx)
			{
				boxx = dim1;
				boxy = dim2;
				boxz = dim3;
				bfx = hmx - dim1;
				bfy = hy - dim2;
				bfz = abs(hz - dim3);
				boxi = x;
			}
			else if (hy - dim2 == bfy && hmx - dim1 == bfx && abs(hz - dim3) < bfz)
			{
				boxx = dim1;
				boxy = dim2;
				boxz = dim3;
				bfx = hmx - dim1;
				bfy = hy - dim2;
				bfz = abs(hz - dim3);
				boxi = x;
			}
		}
		else
		{
			if (dim2 - hy < bbfy)
			{
				bboxx = dim1;
				bboxy = dim2;
				bboxz = dim3;
				bbfx = hmx - dim1;
				bbfy = dim2 - hy;
				bbfz = abs(hz - dim3);
				bboxi = x;
			}
			else if (dim2 - hy == bbfy && hmx - dim1 < bbfx)
			{
				bboxx = dim1;
				bboxy = dim2;
				bboxz = dim3;
				bbfx = hmx - dim1;
				bbfy = dim2 - hy;
				bbfz = abs(hz - dim3);
				bboxi = x;
			}
			else if (dim2 - hy == bbfy && hmx - dim1 == bbfx && abs(hz - dim3) < bbfz)
			{
				bboxx = dim1;
				bboxy = dim2;
				bboxz = dim3;
				bbfx = hmx - dim1;
				bbfy = dim2 - hy;
				bbfz = abs(hz - dim3);
				bboxi = x;
			}
		}
	}
}

//----------------------------------------------------------------------------
// FINDS THE FIRST TO BE PACKED GAP IN THE LAYER EDGE
//----------------------------------------------------------------------------

void find_smallest_z(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	scrapmemb = scrapfirst;
	smallestz = scrapmemb;
	while (!(scrapmemb->next == NULL))
	{
		if (scrapmemb->next->cumz < smallestz->cumz)
		{
			smallestz = scrapmemb->next;
		}
		scrapmemb = scrapmemb->next;
	}
	return;
}

//----------------------------------------------------------------------------
// AFTER FINDING EACH BOX, THE CANDIDATE BOXES AND THE CONDITION OF THE LAYER
// ARE EXAMINED
//----------------------------------------------------------------------------

void checkfound(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	evened = 0;
	if (boxi)
	{
		cboxi = boxi;
		cboxx = boxx;
		cboxy = boxy;
		cboxz = boxz;
	}
	else
	{
		if ((bboxi > 0) && (layerinlayer || (!smallestz->prev && !smallestz->next)))
		{
			if (!layerinlayer)
			{
				prelayer = layerthickness;
				lilz = smallestz->cumz;
			}
			cboxi = bboxi;
			cboxx = bboxx;
			cboxy = bboxy;
			cboxz = bboxz;
			layerinlayer = layerinlayer + bboxy - layerthickness;
			layerthickness = bboxy;
		}
		else
		{
			if (!smallestz->prev && !smallestz->next)
			{
				layerdone = 1;
			}
			else
			{
				evened = 1;
				if (!smallestz->prev)
				{
					trash = smallestz->next;
					smallestz->cumx = smallestz->next->cumx;
					smallestz->cumz = smallestz->next->cumz;
					smallestz->next = smallestz->next->next;
					if (smallestz->next)
					{
						smallestz->next->prev = smallestz;
					}
					free(trash);
				}
				else if (!smallestz->next)
				{
					smallestz->prev->next = NULL;
					smallestz->prev->cumx = smallestz->cumx;
					free(smallestz);
				}
				else
				{
					if (smallestz->prev->cumz == smallestz->next->cumz)
					{
						smallestz->prev->next = smallestz->next->next;
						if (smallestz->next->next)
						{
							smallestz->next->next->prev = smallestz->prev;
						}
						smallestz->prev->cumx = smallestz->next->cumx;
						free(smallestz->next);
						free(smallestz);
					}
					else
					{
						smallestz->prev->next = smallestz->next;
						smallestz->next->prev = smallestz->prev;
						if (smallestz->prev->cumz < smallestz->next->cumz)
						{
							smallestz->prev->cumx = smallestz->cumx;
						}
						free(smallestz);
					}
				}
			}
		}
	}
	return;
}

//----------------------------------------------------------------------------
// AFTER PACKING OF EACH BOX, 100% PACKING CONDITION IS CHECKED
//----------------------------------------------------------------------------

void volume_check(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	boxlist[cboxi].is_packed = TRUE;
	boxlist[cboxi].packx = cboxx;
	boxlist[cboxi].packy = cboxy;
	boxlist[cboxi].packz = cboxz;
	packedvolume = packedvolume + boxlist[cboxi].vol;
	packednumbox++;
	if (packingbest)
	{
		write_visualization_data_file();
		write_boxlist_file();
	}
	else if (packedvolume == total_pallet_volume || packedvolume == total_box_volume)
	{
		packing = 0;
		hundredpercent = 1;
	}
	return;
}

//----------------------------------------------------------------------------
// DATA FOR THE VISUALIZATION PROGRAM IS WRITTEN TO THE "VISUDAT" FILE AND THE
// LIST OF UNPACKED BOXES IS MERGED TO THE END OF THE REPORT FILE
//----------------------------------------------------------------------------

void write_visualization_data_file(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	char n[5];
	if (!unpacked)
	{
		sprintf(strcox, "%d", boxlist[cboxi].cox);
		sprintf(strcoy, "%d", boxlist[cboxi].coy);
		sprintf(strcoz, "%d", boxlist[cboxi].coz);
		sprintf(strpackx, "%d", boxlist[cboxi].packx);
		sprintf(strpacky, "%d", boxlist[cboxi].packy);
		sprintf(strpackz, "%d", boxlist[cboxi].packz);
	}
	else
	{
		sprintf(n, "%d", cboxi);
		sprintf(strpackx, "%d", boxlist[cboxi].dim1);
		sprintf(strpacky, "%d", boxlist[cboxi].dim2);
		sprintf(strpackz, "%d", boxlist[cboxi].dim3);
	}
	if (!unpacked)
	{
		fprintf(visualizer_file, "[%5s, %5s, %5s, %5s, %5s, %5s],\n ", strcox, strcoy, strcoz, strpackx, strpacky, strpackz);
	}
	else
	{
		fprintf(report_output_file, "%5s%5s%5s%5s\n", n, strpackx, strpacky, strpackz);
	}
}

//----------------------------------------------------------------------------
// TRANSFORMS THE FOUND COORDINATE SYSTEM TO THE ONE ENTERED BY THE USER AND
// WRITES THEM TO THE REPORT FILE
//----------------------------------------------------------------------------

void write_boxlist_file(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	char strx[5];
	char strpackst[5];
	char strdim1[5], strdim2[5], strdim3[5];
	char strcox[5], strcoy[5], strcoz[5];
	char strpackx[5], strpacky[5], strpackz[5];

	short int x, y, z, bx, by, bz;

	switch (best_variant)
	{
	case 1:
		x = boxlist[cboxi].cox;
		y = boxlist[cboxi].coy;
		z = boxlist[cboxi].coz;
		bx = boxlist[cboxi].packx;
		by = boxlist[cboxi].packy;
		bz = boxlist[cboxi].packz;
		break;
	case 2:
		x = boxlist[cboxi].coz;
		y = boxlist[cboxi].coy;
		z = boxlist[cboxi].cox;
		bx = boxlist[cboxi].packz;
		by = boxlist[cboxi].packy;
		bz = boxlist[cboxi].packx;
		break;
	case 3:
		x = boxlist[cboxi].coy;
		y = boxlist[cboxi].coz;
		z = boxlist[cboxi].cox;
		bx = boxlist[cboxi].packy;
		by = boxlist[cboxi].packz;
		bz = boxlist[cboxi].packx;
		break;
	case 4:
		x = boxlist[cboxi].coy;
		y = boxlist[cboxi].cox;
		z = boxlist[cboxi].coz;
		bx = boxlist[cboxi].packy;
		by = boxlist[cboxi].packx;
		bz = boxlist[cboxi].packz;
		break;
	case 5:
		x = boxlist[cboxi].cox;
		y = boxlist[cboxi].coz;
		z = boxlist[cboxi].coy;
		bx = boxlist[cboxi].packx;
		by = boxlist[cboxi].packz;
		bz = boxlist[cboxi].packy;
		break;
	case 6:
		x = boxlist[cboxi].coz;
		y = boxlist[cboxi].cox;
		z = boxlist[cboxi].coy;
		bx = boxlist[cboxi].packz;
		by = boxlist[cboxi].packx;
		bz = boxlist[cboxi].packy;
		break;
	}

	sprintf(strx, "%d", cboxi);
	sprintf(strpackst, "%d", boxlist[cboxi].is_packed);
	sprintf(strdim1, "%d", boxlist[cboxi].dim1);
	sprintf(strdim2, "%d", boxlist[cboxi].dim2);
	sprintf(strdim3, "%d", boxlist[cboxi].dim3);
	sprintf(strcox, "%d", x);
	sprintf(strcoy, "%d", y);
	sprintf(strcoz, "%d", z);
	sprintf(strpackx, "%d", bx);
	sprintf(strpacky, "%d", by);
	sprintf(strpackz, "%d", bz);

	boxlist[cboxi].cox = x;
	boxlist[cboxi].coy = y;
	boxlist[cboxi].coz = z;
	boxlist[cboxi].packx = bx;
	boxlist[cboxi].packy = by;
	boxlist[cboxi].packz = bz;
	fprintf(report_output_file, "%5s%5s%9s%9s%9s%9s%9s%9s%9s%9s%9s\n", strx, strpackst, strdim1, strdim2, strdim3, strcox, strcoy, strcoz, strpackx, strpacky, strpackz);
	return;
}

//----------------------------------------------------------------------------
// USING THE PARAMETERS FOUND, PACKS THE BEST SOLUTION FOUND AND REPORS TO THE
// CONSOLE AND TO A TEXT FILE
//----------------------------------------------------------------------------

void report_results(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
	switch (best_variant)
	{
	case 1:
		pallet_x = xx; pallet_y = yy; pallet_z = zz;
		break;
	case 2:
		pallet_x = zz; pallet_y = yy; pallet_z = xx;
		break;
	case 3:
		pallet_x = zz; pallet_y = xx; pallet_z = yy;
		break;
	case 4:
		pallet_x = yy; pallet_y = xx; pallet_z = zz;
		break;
	case 5:
		pallet_x = xx; pallet_y = zz; pallet_z = yy;
		break;
	case 6:
		pallet_x = yy; pallet_y = zz; pallet_z = xx;
		break;
	}
	packingbest = 1;
	if ((visualizer_file = fopen(graphout, "w")) == NULL)
	{
		printf("Cannot open file %s\n", filename);
		exit(1);
	}

	sprintf(strpx, "%d", pallet_x);
	sprintf(strpy, "%d", pallet_y);
	sprintf(strpz, "%d", pallet_z);

	// Some terrible json formatter
	fprintf(visualizer_file, "{\n \"pallet\" : {\n \"pallet_x\" : %5s,\n \"pallet_y\" : %5s,\n \"pallet_z\" : %5s\n },\n \"boxes\" : [\n ", strpx, strpy, strpz);
	strcat(filename, ".out");

	if ((report_output_file = fopen(filename, "w")) == NULL)
	{
		printf("Cannot open output file %s\n", filename);
		exit(1);
	}

	packed_box_percentage = best_solution_volume * 100 / total_box_volume;
	pallet_volume_used_percentage = best_solution_volume * 100 / total_pallet_volume;
	elapsed_time = difftime(finish, start);

	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");
	fprintf(report_output_file, "                                       *** REPORT ***\n");
	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");
	fprintf(report_output_file, "ELAPSED TIME                                          : Almost %.0f sec\n", elapsed_time);
	fprintf(report_output_file, "TOTAL NUMBER OF ITERATIONS DONE                       : %d\n", number_of_iterations);
	fprintf(report_output_file, "BEST SOLUTION FOUND AT ITERATION                      : %d OF VARIANT: %d\n", best_iteration, best_variant);
	fprintf(report_output_file, "TOTAL NUMBER OF BOXES                                 : %d\n", total_boxes);
	fprintf(report_output_file, "PACKED NUMBER OF BOXES                                : %d\n", number_packed_boxes);
	fprintf(report_output_file, "TOTAL VOLUME OF ALL BOXES                             : %.0f\n", total_box_volume);
	fprintf(report_output_file, "PALLET VOLUME                                         : %.0f\n", total_pallet_volume);
	fprintf(report_output_file, "BEST SOLUTION'S VOLUME UTILIZATION                    : %.0f OUT OF %.0f\n", best_solution_volume, total_pallet_volume);
	fprintf(report_output_file, "PERCENTAGE OF PALLET VOLUME USED                      : %.6f %%\n", pallet_volume_used_percentage);
	fprintf(report_output_file, "PERCENTAGE OF PACKED BOXES (VOLUME)                   : %.6f %%\n", packed_box_percentage);
	fprintf(report_output_file, "WHILE PALLET ORIENTATION                              : X = %d; Y = %d; Z = %d\n", pallet_x, pallet_y, pallet_z);
	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");
	fprintf(report_output_file, "  NO: PACKSTA DIMEN-1  DMEN-2  DIMEN-3   COOR-X   COOR-Y   COOR-Z   PACKEDX  PACKEDY  PACKEDZ\n");
	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");

	list_candidate_layers();
	layers[0].layereval = -1;
	qsort(layers, layerlistlen + 1, sizeof(struct layerlist), compute_layer_list);
	packedvolume = 0.0;
	packedy = 0;
	packing = 1;
	layerthickness = layers[best_iteration].layerdim;
	remainpy = pallet_y;
	remainpz = pallet_z;

	for (x = 1; x <= total_boxes; x++)
	{
		boxlist[x].is_packed = FALSE;
	}

	do
	{
		layerinlayer = 0;
		layerdone = 0;
		pack_layer();
		packedy = packedy + layerthickness;
		remainpy = pallet_y - packedy;
		if (layerinlayer)
		{
			prepackedy = packedy;
			preremainpy = remainpy;
			remainpy = layerthickness - prelayer;
			packedy = packedy - layerthickness + prelayer;
			remainpz = lilz;
			layerthickness = layerinlayer;
			layerdone = 0;
			pack_layer();
			packedy = prepackedy;
			remainpy = preremainpy;
			remainpz = pallet_z;
		}
		find_layer(remainpy);
	} while (packing);

	fprintf(report_output_file, "\n\n *** LIST OF UNPACKED BOXES ***\n");
	unpacked = 1;

	for (cboxi = 1; cboxi <= total_boxes; cboxi++)
	{
		if (!boxlist[cboxi].is_packed)
		{
			write_visualization_data_file();
		}
	}

	unpacked = 0;
	fclose(report_output_file);
	fprintf(visualizer_file, "[]\n ]\n}");
	fclose(visualizer_file);
	printf("\n");
	for (n = 1; n <= total_boxes; n++)
	{
		if (boxlist[n].is_packed)
		{
			printf("%d. %d %d %d; %d %d %d; %d %d %d\n", n, boxlist[n].dim1, boxlist[n].dim2, boxlist[n].dim3, boxlist[n].cox, boxlist[n].coy, boxlist[n].coz, boxlist[n].packx, boxlist[n].packy, boxlist[n].packz);
		}
	}
	printf("ELAPSED TIME                       : Almost %.0f sec\n", elapsed_time);
	printf("TOTAL NUMBER OF ITERATIONS DONE    : %d\n", number_of_iterations);
	printf("BEST SOLUTION FOUND AT             : ITERATION: %d OF VARIANT: %d\n", best_iteration, best_variant);
	printf("TOTAL NUMBER OF BOXES              : %d\n", total_boxes);
	printf("PACKED NUMBER OF BOXES             : %d\n", number_packed_boxes);
	printf("TOTAL VOLUME OF ALL BOXES          : %.0f\n", total_box_volume);
	printf("PALLET VOLUME                      : %.0f\n", total_pallet_volume);
	printf("BEST SOLUTION'S VOLUME UTILIZATION : %.0f OUT OF %.0f\n", best_solution_volume, total_pallet_volume);
	printf("PERCENTAGE OF PALLET VOLUME USED   : %.6f %%\n", pallet_volume_used_percentage);
	printf("PERCENTAGE OF PACKED BOXES (VOLUME): %.6f%%\n", packed_box_percentage);
	printf("WHILE PALLET ORIENTATION           : X = %d; Y = %d; Z = %d\n", pallet_x, pallet_y, pallet_z);
}
}