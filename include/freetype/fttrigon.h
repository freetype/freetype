#ifndef __FT_TRIGONOMETRY_H__
#define __FT_TRIGONOMETRY_H__

FT_BEGIN_HEADER

 /***************************************************************************
  *
  * @section: computations
  *
  */

 /***************************************************************************
  *
  * @type: FT_Angle
  *
  * @description:
  *   this type is used to model angle values in FreeType. Note that
  *   the angle is a 16.16 fixed float value expressed in _degrees_
  */
  typedef FT_Fixed   FT_Angle;

 /***************************************************************************
  *
  * @macro: FT_ANGLE_PI
  *
  * @description:
  *   the angle pi expressed in @FT_Angle units
  */
#define  FT_ANGLE_PI   (180L << 16)

 /***************************************************************************
  *
  * @macro: FT_ANGLE_2PI
  *
  * @description:
  *   the angle 2pi expressed in @FT_Angle units
  */
#define  FT_ANGLE_2PI  (FT_ANGLE_PI*2)

 /***************************************************************************
  *
  * @macro: FT_ANGLE_PI2
  *
  * @description:
  *   the angle pi/2 expressed in @FT_Angle units
  */
#define  FT_ANGLE_PI2  (FT_ANGLE_PI/2)

 /***************************************************************************
  *
  * @macro: FT_ANGLE_PI4
  *
  * @description:
  *   the angle pi/4 expressed in @FT_Angle units
  */
#define  FT_ANGLE_PI4  (FT_ANGLE_PI/4)


 /***************************************************************************
  *
  * @function: FT_Sin
  *
  * @description:
  *   return the sinus of a given angle in fixed point format
  *
  * @input:
  *    angle :: input angle
  *
  * @return:
  *    sinus value
  *
  * @note:
  *   if you need both the sinus and cosinus for a given angle, you'd
  *   better use the function @FT_Vector_Unit
  */
  FT_EXPORT(FT_Fixed)    FT_Sin( FT_Angle  angle );

 /***************************************************************************
  *
  * @function: FT_Cos
  *
  * @description:
  *   return the cosinus of a given angle in fixed point format
  *
  * @input:
  *    angle :: input angle
  *
  * @return:
  *    cosinus value
  *
  * @note:
  *   if you need both the sinus and cosinus for a given angle, you'd
  *   better use the function @FT_Vector_Unit
  */
  FT_EXPORT(FT_Fixed)    FT_Cos( FT_Angle  angle );  

 /***************************************************************************
  *
  * @function: FT_Tan
  *
  * @description:
  *   return the tangent of a given angle in fixed point format
  *
  * @input:
  *   angle :: input angle
  *
  * @return:
  *   tangent value
  */
  FT_EXPORT(FT_Fixed)    FT_Tan( FT_Angle  angle );


 /***************************************************************************
  *
  * @function: FT_Atan2
  *
  * @description:
  *   return the arc-tangent corresponding to a given vector (x,y) in
  *   the 2d plane
  *
  * @input:
  *   x :: horizontal vector coordinate
  *   y :: vertical vector coordinate
  *
  * @return:
  *   arc-tangent value (i.e. angle)
  */
  FT_EXPORT(FT_Angle)    FT_Atan2( FT_Fixed  x, FT_Fixed  y );


 /***************************************************************************
  *
  * @function: FT_Vector_Unit
  *
  * @description:
  *   return the unit vector corresponding to a given angle. After the call,
  *   the value of "vec.x" will be "sin(theta)", and the value of "vec.y"
  *   will be "cos(angle)"
  *
  *   this function is useful to retrieve both the sinus and cosinus
  *   of a given angle quickly
  *
  * @input:
  *   vec   :: address of target vector
  *   angle :: address of angle
  */
  FT_EXPORT(void)        FT_Vector_Unit( FT_Vector*  vec,
                                         FT_Angle    angle );

 /***************************************************************************
  *
  * @function: FT_Vector_Rotate
  *
  * @description:
  *   rotate a given vector by a given angle
  *
  * @input:
  *   vec   :: address of target vector
  *   angle :: address of angle
  */
  FT_EXPORT(void)        FT_Vector_Rotate( FT_Vector*    vec,
                                           FT_Angle      angle );

 /***************************************************************************
  *
  * @function: FT_Vector_Length
  *
  * @description:
  *   returns the length of a given vector
  *
  * @input:
  *   vec   :: address of target vector
  *
  * @return:
  *   vector length, expressed in the same units that the original
  *   vector coordinates !!
  */
  FT_EXPORT(FT_Fixed)    FT_Vector_Length( FT_Vector*  vec );

 /***************************************************************************
  *
  * @function: FT_Vector_Normalize
  *
  * @description:
  *   normalize a given vector (i.e. compute the equivalent unit vector)
  *
  * @input:
  *   vec   :: address of target vector
  */
  FT_EXPORT(void)        FT_Vector_Normalize( FT_Vector*  vec );

 /***************************************************************************
  *
  * @function: FT_Vector_Polarize
  *
  * @description:
  *   compute both the length and angle of a given vector
  *
  * @input:
  *   vec    :: address of source vector
  *
  * @output:
  *   length :: vector length
  *   angle  :: vector angle
  */
  FT_EXPORT(void)        FT_Vector_Polarize( FT_Vector*  vec,
                                             FT_Fixed   *length,
                                             FT_Angle   *angle );
  /* */
  
FT_END_HEADER

#endif /* __FT_TRIGONOMETRY_H__ */
