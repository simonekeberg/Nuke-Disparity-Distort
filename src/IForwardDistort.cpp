// IForwardDistort.C
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \class IForwardDistort IForwardDistort.C

   This class implements a plug-in to the DDImage library that warps one image
   based on an image of u, v vector coordinates. Each pixel in the destination
   image has a vector [u, v] associated with it. This vector tells the image
   where to pull it's pixel from the input image.

   This class started out as the VectorBlur class.

   \author Doug Roble
   \date May 2nd, 2001  File creation.
 */

// Standard plug-in include files.

#include <stdio.h>
#include <stdlib.h>
#include "DDImage/Iop.h"
#include "DDImage/NukeWrapper.h"
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Pixel.h"
#include "DDImage/Filter.h"
#include "DDImage/Knobs.h"
#include "DDImage/Vector2.h"
#include "DDImage/DDMath.h"

using namespace DD::Image;

static const char* const CLASS = "IForwardDistort";
static const char* const HELP = "IForwardDistort: Same as IDistort but using a forward seeking algorithm. v0.1";


class IForwardDistort : public Iop
{
  Channel disparity;
  Channel alpha_channel;
  Filter filter;
  double disparity_scale, disparity_near, sample_area;
  int disparity_limit[2];
  bool disparity_enable;

public:

  IForwardDistort (Node* node) : Iop (node)
  {
    disparity = Chan_Black;
    alpha_channel = Chan_Black;
    disparity_near = .5;
    disparity_scale = 1;
    disparity_limit[0] = -60;
    disparity_limit[1] = 60;
    disparity_enable = true;
    sample_area = 1.0;
  }

  ~IForwardDistort () { }

  void _validate(bool)
  {
    filter.initialize();
    copy_info();
  }

  void in_channels(int, ChannelSet& m) const
  {
    m += (disparity);
    m += (alpha_channel);
  }

  void _request(int x, int y, int r, int t, ChannelMask channels, int count)
  {
    ChannelSet c1(channels);
    in_channels(0, c1);
    input0().request(input0().info().x(),
                     input0().info().y(),
                     input0().info().r(),
                     input0().info().t(),
                     c1,
                     count * 2);
  }

  void engine ( int y, int x, int r, ChannelMask channels, Row& out );

  virtual void knobs ( Knob_Callback f )
  {
    Input_Channel_knob ( f, &disparity, 1, 0, "disparity", "Disparity channel");
    //Input_Channel_knob(f, &alpha_channel, 1, 0, "maskChannel", "mask channel");

    Double_knob(f, &disparity_scale, IRange(0, 1), "disparity_scale", "Disparity scale");

    Double_knob(f, &disparity_near, IRange(0, 3), "disparity_near", "Disparity near threshold");
    Tooltip(f, "Keep around .1->1 depending on disparity.");

    MultiInt_knob(f, disparity_limit, 2, "disparity_limit", "Disparity limit");
    Tooltip(f, "The limit in pixels how far the plugin searches for neighbouring disparity."
               "The first value is how many pixels to the left of the current pixel, and the second is to the right"
               "First value should always be negative and the the second positive!"
               "As an optimization, you could set one of the values to zero, depending of if your disparity is from left->right or right->left.");

    filter.knobs(f);
    Double_knob(f, &sample_area, IRange(0, 3), "sample_area", "Sample area");
  }

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }
  static const Iop::Description description;

};


static Iop* IForwardDistortCreate(Node* node) {
  return (new NukeWrapper (new IForwardDistort(node)))->noMix()->noMask();
}

const Iop::Description IForwardDistort::description ( CLASS, 0, IForwardDistortCreate );


void IForwardDistort::engine ( int y, int x, int r, ChannelMask channels, Row& out )
{
    ChannelSet c1(channels);
    in_channels(0, c1);

    Tile tile(input0(), x, y, r + 1, y + 2, c1);

    if (aborted()) return;

    Channel di = disparity;
    if (!intersect(tile.channels(), di))
    di = Chan_Black;


    const float d_scale = float(this->disparity_scale);
    const float d_near = float(this->disparity_near);
    const float s_area = float(this->sample_area);
    const int d_limit1 = this->disparity_limit[0];
    const int d_limit2 = this->disparity_limit[1];


    // Get pointers to the various channels:
    const float* const D = tile[di][y];

    foreach(z, channels) out.writable(z);

    InterestRatchet interestRatchet;
    Pixel pixel(channels);
    pixel.setInterestRatchet(&interestRatchet);

    //Loop all pixels in current row
    for (; x < r; x++) {

        if ( aborted() ) break;

        if(d_scale > 0.0) {

            bool dist_found = false;
            float dist, dist_cur=0;
            int xx;

            for(xx = tile.clampx(x+d_limit1); xx< tile.clampx(x+d_limit2);xx++ ) {

                dist = D[xx] - ((float)x-xx);

                if( fabs(dist) < d_near ){

                    if(dist_found && fabs(dist_cur) < fabs(D[xx]) ) {
                        dist_cur = D[xx];
                        dist_found = true;
                    }else if(!dist_found){
                        dist_found = true;
                        dist_cur = D[xx];
                    }
                }
            }

            if(dist_found )
                input0().sample(x+.5 - dist_cur*d_scale, y+.5, s_area , s_area, &filter, pixel);

            foreach (z, channels) {
                if(dist_found)
                    ((float*)(out[z]))[x] = pixel[z];
                else 
                    ((float*)(out[z]))[x] = 0.0;
            }

        }else {
            input0().sample(x+.5, y+.5, 1 , 1, &filter, pixel);
            foreach (z, channels) 
                ((float*)(out[z]))[x] = pixel[z];
        }
    }



}



