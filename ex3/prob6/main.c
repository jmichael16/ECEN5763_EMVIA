/* ---------------------------------------------------------------------------- 
 * @file main.c
 * @brief Laser tracking in light room with clutter
 *
 * @author Jake Michael, jami1063@colorado.edu
 * @course ECEN 5763: EMVIA, Summer 2021
 *---------------------------------------------------------------------------*/

#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include"imageio.h"
#include"imageproc.h"

#define INPUT_FOLDER    ("PPM_in")
#define OUTPUT_FOLDER   ("PPM_out")
#define FILENAME_LEN    (30)
#define BG_UPDATE       (10)    // frames to diff before updating bg 
#define STARTFRAME      (1)    // starting frame no
#define NFRAMES         (1740)  // total num frames to analyze
//#define NFRAMES         (100)  // total num frames to analyze

// 
// the main application
//
int main(int argc, char** argv)  {

  ppm_img_t *bg = NULL;
  ppm_img_t *fr = NULL;
  ppm_img_t *fr_copy = NULL;
  ppm_img_t *diff_filt = NULL;
  ppm_img_t *diff = NULL;
  int xbar, ybar;
  pixl_t color = {255, 0, 0};
  pixl_t thresh = {40, 40, 40};

  char filename[FILENAME_LEN];
  int i=STARTFRAME;

  // read first image (set background)
  sprintf(filename, "%s/DRLS%04d.ppm", INPUT_FOLDER, i);
  bg = read_ppm(filename);
  if (!bg) {
    printf("bg ptr is NULL");
    return -1;
  }

  for (i=STARTFRAME+1; i<STARTFRAME+NFRAMES; i++) {

    // read input frame
    sprintf(filename, "%s/DRLS%04d.ppm", INPUT_FOLDER, i);
    fr = read_ppm(filename);
    fr_copy = copy_ppm(fr);
    if (!fr) {
      printf("bg ptr is NULL");
      return -1;
    }

    // do rgb differencing
    diff = rgb_diff(fr, bg);
    if (!diff) {
      printf("rgb_diff fail %d\n", i);
      return -1;
    }

    // apply a median filter to 3-channel rgb
    diff_filt = median_filter_rgb(diff);
    if (!diff_filt) {
      printf("median filter error\n"); 
      return -1;
    }

    // detect COM + annotate
    if (com_rgb(diff_filt, thresh, &xbar, &ybar)) {
      printf("com_rgb error\n");
      return -1;
    }
    if (com_annotate_rgb(fr_copy, color, xbar, ybar)) {
      printf("com_annotate_rgb error\n");
      return -1;
    }
    sprintf(filename, "%s/DRLS%04d_out.ppm", OUTPUT_FOLDER, i);
    if (write_ppm(filename, fr_copy)) {
      printf("write_ppm fail\n");
      return -1;
    }
    
    // deallocate unused heap vars
    yeet_ppm(&fr_copy);
    yeet_ppm(&diff);
    yeet_ppm(&diff_filt);

    // update background every BG_UPDATE frames
    if ( ((i-1) % BG_UPDATE) == 0) {
      yeet_ppm(&bg);
      bg = fr;
    } else {
      yeet_ppm(&fr);
    }

  }
  
} // end main

