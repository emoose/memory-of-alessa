#include "vc_util.h"
#include "SH2_common/sh_vu0.h"
#include "SH2_common/sh2sys.h"
#include "common.h"
#include "vec.h"
#include "vb_main.h"
#include "vw_main.h"

#line 63
void vcInitCamera(VC_ROAD_DATA** roadarray_list) {

    vcCameraInternalInfo.mode = 0;
    vcCameraInternalInfo.mv_smooth = 0;
    vcCameraInternalInfo.ev_cam_rate = 0.0f;


    vcPreSetCharaDataForCamera();

    vcSetCameraUseWarp(sys.hero.pos, sys.hero.ang[1]);

    vwInitViewInfo();

    vcInitVCSystem(roadarray_list);



    vcStartCameraSystem();
    vcSetProjectionValue(0.0f, 0);

    vcWork.flags |= VC_PROJ_MOMENT_CHANGE_F;



    sys.cam_ang_z = 0.0f;
    sys.cam_r_xz = 1500.0f;
    sys.cam_y = 0.0f;
}

#line 108
void vcSetCameraUseWarp(sceVu0FVECTOR chr_pos, float chr_ang_y) {
    sceVu0FVECTOR cam_pos;
    sceVu0FVECTOR cam_ang;


    vec_zero(cam_ang); // @bug: Arrays are set but never used, vec_zero uses asm that makes compiler forget this?
    cam_ang[1] = chr_ang_y;


    cam_pos[0] = chr_pos[0] - (795.0f * shSinF(chr_ang_y));
    cam_pos[1] = chr_pos[1] + -900.0f;
    cam_pos[2] = chr_pos[2] - (795.0f * shCosF(chr_ang_y));
    cam_pos[3] = 1.0f;


    vcSetFirstCamWork(chr_pos, chr_ang_y, GET_BIT(Sh2sys.main_status, 2));
    UNSET_BIT(Sh2sys.main_status, 2);
}

#line 142
int vcRetCamMvSmoothF(void) {
    return vcCameraInternalInfo.mv_smooth;
}

// @hack: adding unused static inline asm func here fixes some float arg issues inside `vcMoveAndSetCamera`.
// Copied from `vec_zero`.
// Possible there were other VC functions here that went unused and got removed by DCE, but still affected floats?
// (60+ lines between these two funcs, SH1 also has some other funcs living between these)
static inline void vec_zero_hack(void* x) { asm("sq zero, 0(%0)" : "+r"(x)); }

#line 210
void vcMoveAndSetCamera(int in_connect_f, int change_debug_mode, int for_f, int back_f, int right_f, int left_f, int up_f, int down_f)
{
    sceVu0FVECTOR first_cam_pos;
    KANRI* hr_p;
    sceVu0FVECTOR hr_head_pos;
    float hero_bottom_y;
    float hero_top_y;
    float grnd_y;
    VbCOORDINATE vbcoord;
    sceVu0FVECTOR rpos;























    
    vcPreSetCharaDataForCamera();











    
    switch (vcCameraInternalInfo.mode) {
        default:


            
            vcCameraInternalInfo.mode = 0;
            
            first_cam_pos[0] = sys.hero.pos[0] + 3500.0f;
            first_cam_pos[1] = -1100.0f;
            first_cam_pos[2] = sys.hero.pos[2];
            first_cam_pos[3] = 1.0f;
            vcSetFirstCamWork(first_cam_pos, sys.hero.ang[1], 0);
        case 0:








            
            hr_p = &sys.hero;
            
            if (in_connect_f) {
                hr_head_pos[0] = hr_p->pos[0];
                hr_head_pos[1] = hr_p->pos[1] + -950.0f;
                hr_head_pos[2] = hr_p->pos[2];
                hr_head_pos[3] = 1.0f;
                grnd_y = -2.0f;
                
            } else {
                grnd_y = hr_p->pos[1];
                
                vcMakeHeroHeadPos(hr_head_pos);
            }
            
            hero_top_y = hr_p->pos[1] - 925.0f;
            hero_bottom_y = hr_p->pos[1] + (-250.0f * vcCameraInternalInfo.ev_cam_rate);

            
            if (vcCameraInternalInfo.ev_cam_rate > 0.0f)
                vcWork.flags |= VC_INHIBIT_FAR_WATCH_F;
            else
                vcWork.flags &= ~VC_INHIBIT_FAR_WATCH_F;
            
            
            
            vcSetSubjChara(hr_p->pos, 
                           hero_bottom_y, 
                           hero_top_y, 
                           grnd_y, 
                           hr_head_pos, 
                           hr_p->velo_xz, 
                           hr_p->velo_houi, 
                           hr_p->rot_spd[1], 
                           hr_p->ang[1], 
                           TO_RAD(120.0f),
                           5500.0f);
            
            vcCameraInternalInfo.mv_smooth = vcExecCamera();
            
            break;
        
        case 1:
            vcSetRefPosAndSysRef2CamParam(&vcRefPosSt, &sys, 
                                          for_f, back_f, 
                                          right_f, left_f, up_f, down_f);
            
            vwSetCoordRefAndEntou(NULL, &vcRefPosSt, 
                                  sys.cam_ang_y, sys.cam_ang_z, 
                                  sys.cam_y, sys.cam_r_xz);
            
            break;
        
        case 2:
            vcSetRefPosAndCamPosAngByPad(&vcRefPosSt, &sys);
            break;
        case 3:
            vcSetRefPosAndSysRef2CamParam(&vcRefPosSt, 
                                          &sys, 
                                          for_f, 
                                          back_f, 
                                          right_f, 
                                          left_f, 
                                          up_f, 
                                          down_f);









            
            mat_copy(vbcoord.work, vcPreInfo.hero_neck_wm);
            mat_copy(vbcoord.coord, vcPreInfo.hero_neck_lm);
            mat_copy(vbcoord.lw, vcPreInfo.hero_neck_lwm);

            vu0_unit_vector(rpos);
            
            rpos[1] = -75.0f;
            
            rpos[2] = 500.0f;

            
            vwSetCoordRefAndEntou(&vbcoord, rpos, TO_RAD(165.0f), TO_RAD(0.0f), -100, 500);
            break;
    }




    
    vwSetViewInfo();






































}

#line 429
void vcMakeHeroHeadPos(sceVu0FVECTOR head_pos) {
    sceVu0FMATRIX neck_lwm;
    sceVu0FVECTOR fpos;



    mat_copy(neck_lwm, vcPreInfo.hero_neck_lwm);

    vec_zero(fpos);
    fpos[1] = -50.0f;
    vbApplyMatrixWithoutTr(fpos, neck_lwm, fpos);



    vec_add_reverse(neck_lwm[3], fpos, head_pos);
    head_pos[1] += -150.0f;
    head_pos[3] = 1.0f;
}

#line 468
void vcAddOfsToPos(sceVu0FVECTOR out_pos, sceVu0FVECTOR in_pos, float ofs_xz_r, float ang_y, float ofs_y) {
    out_pos[0] = in_pos[0] + (ofs_xz_r * shSinF(ang_y));
    out_pos[2] = in_pos[2] + (ofs_xz_r * shCosF(ang_y));
    out_pos[1] = in_pos[1] + ofs_y;
}

#line 497
void vcSetRefPosAndSysRef2CamParam(sceVu0FVECTOR ref_pos, SYS_W* sys_p, int for_f, int back_f, int right_f, int left_f, int up_f, int down_f) {






    if (for_f) sys_p->cam_r_xz -= 50.0f;
    if (back_f) sys_p->cam_r_xz += 50.0f;
    if (right_f) sys_p->cam_ang_y -= TO_RAD(1.0f);
    if (left_f) sys_p->cam_ang_y += TO_RAD(1.0f);
    if (up_f) sys_p->cam_y -= 50.0f;
    if (down_f) sys_p->cam_y += 50.0f;

    sys_p->cam_ang_y = shAngleRegulate(sys_p->cam_ang_y);

    if (sys_p->cam_r_xz < 500.0f) sys_p->cam_r_xz = 500.0f;


    vcAddOfsToPos(ref_pos, sys.hero.pos, 250.0f, sys.hero.ang[1], -500.0f);


}

// @note: arguments missing from symbols, maybe caused by commented out code?
#line 539
void vcSetRefPosAndCamPosAngByPad(sceVu0FVECTOR ref_pos, SYS_W* sys_p) {
    sceVu0FVECTOR cam_ang;
    sceVu0FVECTOR cam_pos;

    vwGetViewPosition(cam_pos);
    vec_div_xyz(cam_pos, 500.0f, cam_pos);

    vwGetViewAngle(cam_ang);

    #line 623 // line 547 -> 623? Commented-out code? SH1 version of this func does a lot more.
}
