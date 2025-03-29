#include "RemoSystemYX.h"

void ButtonYX::Set_Pos(int x, int y)
{
  x_targ = x;
  y_targ = y;
}

void ButtonYX::Set_Size(int w, int h)
{
  w_targ = w;
  h_targ = h;
}

void ButtonYX::To_Targ_State()
{
  float x_err = x_targ - x_now;
  float y_err = y_targ - y_now;
  float w_err = w_targ - w_now;
  float h_err = h_targ - h_now;

  integral_x += x_err;
  integral_y += y_err;
  integral_w += w_err;
  integral_h += h_err;

  x_now += 0.001 * x_err + 0.01 * integral_x + 0.001 * (x_err - previous_x);
  y_now += 0.001 * y_err + 0.01 * integral_y + 0.001 * (y_err - previous_y);
  w_now += 0.001 * w_err + 0.01 * integral_w + 0.001 * (w_err - previous_w);
  h_now += 0.001 * h_err + 0.01 * integral_h + 0.001 * (h_err - previous_h);

  previous_x = x_err;
  previous_y = y_err;
  previous_w = w_err;
  previous_h = h_err;
}