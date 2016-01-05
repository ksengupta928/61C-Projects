// CS 61C Fall 2015 Project 4

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

// include OpenMP
#if !defined(_MSC_VER)
#include <pthread.h>
#endif
#include <omp.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

/* DO NOT CHANGE ANYTHING ABOVE THIS LINE. */
#include <string.h>
#include <float.h>
/* ORIGINAL
void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{
	calcDepthNaive(depth, left, right, imageWidth, imageHeight, featureWidth, featureHeight, maximumDisplacement);
}

*/
//Test



void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{
	float zero = 0;
	memset(depth, 0, sizeof(float) * imageHeight * imageWidth);
	#pragma omp parallel for collapse(2) private(zero)

		for (int y = featureHeight; y < imageHeight - featureHeight; y++)
		{
			for (int x = featureWidth; x < imageWidth - featureWidth; x++)
			{
				float min_displacement = -1;
				int y_image_x = y * imageWidth + x;
				/* Set the depth to 0 if looking at edge of the image where a feature box cannot fit. */
				// int imgH_fH = imageHeight - featureHeight;
				// int imgW_fW = imageWidth - featureWidth;
				// if ((y < featureHeight) || (y >= imgH_fH) || (x < featureWidth) || (x >= imgW_fW))
				// {
				// 	depth[y_image_x] = 0;
				// 	continue;
				// }

				float minimumSquaredDifference = -1;


				/* Iterate through all feature boxes that fit inside the maximum displacement box.
				   centered around the current pixel. */


 				int dy_lim = (maximumDisplacement < (imageHeight - featureHeight - y)) ? maximumDisplacement : (imageHeight - featureHeight - y - 1);

 				int dx_lim = (maximumDisplacement < (imageWidth - featureWidth - x)) ? maximumDisplacement : (imageWidth - featureWidth - x - 1);
				int dy = ((-maximumDisplacement) >= (featureHeight - y)) ? (-maximumDisplacement) : (featureHeight - y);
				int dx_setter = ((-maximumDisplacement) >= (featureWidth - x)) ? (-maximumDisplacement) : (featureWidth - x);
				for (; dy <= dy_lim; dy++)
				{
					int y_dy = y + dy;

					for (int dx = dx_setter; dx <= dx_lim; dx++)
					{
						int x_dx = x + dx;
						/* Skip feature boxes that dont fit in the displacement box. */
						// if ( y_dy  >= imgH_fH || x_dx >= imgW_fW)
						// {
						// 	continue;
						// }

						float squaredDifference = 0;

						__m128 total_diff = _mm_loadu_ps(&zero);
						__m128 total_rest = _mm_loadu_ps(&zero);
						int rem = (featureWidth + featureWidth +1) % 4;
						for (int boxY = -featureHeight; boxY <= featureHeight; boxY++)
						{
							int leftY_mul = (y + boxY) * imageWidth;
							int rightY_mul = (y_dy + boxY) * imageWidth;


							int boxX = -featureWidth;
							for ( ; boxX <= featureWidth - 4; boxX = boxX + 4)
							{
								int leftX = x + boxX;
								int rightX = x_dx + boxX;
								__m128 leftSide = _mm_loadu_ps(&left[leftY_mul + leftX]);
								__m128 rightSide = _mm_loadu_ps(&right[rightY_mul + rightX]);
								__m128 difference = _mm_sub_ps(leftSide,rightSide);
								difference = _mm_mul_ps(difference, difference);
								total_diff = _mm_add_ps(total_diff, difference);
								//squaredDifference += difference[0] + difference[1] + difference[2] + difference[3];

							}
							int leftX = x + boxX;
							int rightX = x_dx + boxX;
							__m128 rest1 = _mm_loadu_ps(&left[leftY_mul + leftX]);
							__m128 rest2= _mm_loadu_ps(&right[rightY_mul + rightX]);
							__m128 difference_rest = _mm_sub_ps(rest1,rest2);
							difference_rest = _mm_mul_ps(difference_rest, difference_rest);
							total_rest = _mm_add_ps(total_rest, difference_rest);



							// for (; rem >= 0; rem--)
							// {
							// 	int diff = featureWidth - rem;
							// 	int leftX = x + diff;
							// 	int rightX = x + dx + diff;
							// 	float difference_f = left[leftY_mul + leftX] - right[rightY_mul + rightX];
							// 	squaredDifference += difference_f * difference_f;
							// }
						}
						squaredDifference += total_diff[0] + total_diff[1] + total_diff[2] + total_diff[3];
							
							if(rem == 0) {
								
							} else if (rem == 1){
								squaredDifference+=total_rest[0];
							} else if(rem == 2) {
								squaredDifference+=total_rest[0] + total_rest[1];
							} else if(rem == 3) {
								squaredDifference+=total_rest[0] + total_rest[1] + total_rest[2];
							}

						/*
						Check if you need to update minimum square difference.
						This is when either it has not been set yet, the current
						squared displacement is equal to the min and but the new
						displacement is less, or the current squared difference
						is less than the min square difference.
						*/
						float pos_displacement = displacementNaive(dx, dy);
						if ((minimumSquaredDifference == -1) || (minimumSquaredDifference > squaredDifference) || ((minimumSquaredDifference == squaredDifference) && ( pos_displacement < min_displacement)))
						{
							minimumSquaredDifference = squaredDifference;
							min_displacement = pos_displacement;

						}
					}
				}

				/*
				Set the value in the depth map.
				If max displacement is equal to 0, the depth value is just 0.
				*/

				if (minimumSquaredDifference > 0)
				{
						depth[y_image_x] = min_displacement;
						/****** CHECK IF THIS IS FASTER *****/
						//Look into faster inverse from Quak
				}
				// else
				// {
				// 	depth[y_image_x] = 0;
				// }
			}
		}

}
