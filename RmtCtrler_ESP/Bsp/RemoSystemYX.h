#ifndef REMOSYS_YX_H
#define REMOSYS_YX_H

class ButtonYX
{
  public:
    ButtonYX()
    {
      x_now = 0;
      y_now = 0;
      w_now = 20;
      h_now = 20;

      x_targ = 100;
      y_targ = 100;
      w_targ = 20;
      h_targ = 20;
    }
    void Set_Pos(int x, int y);   // 中心位置
    void Set_Size(int w, int h);
    void To_Targ_State();

    float x_now;
    float y_now;
    float w_now;
    float h_now;
  
  private:
    float x_targ;
    float y_targ;
    float w_targ;
    float h_targ;

    float integral_x;
    float integral_y;
    float integral_w;
    float integral_h;

    float previous_x;
    float previous_y;
    float previous_w;
    float previous_h;
};



#endif