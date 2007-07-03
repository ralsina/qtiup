#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <string.h>

#include "sim.h"

/*************************************************************************************/
/*                          CLIPPING (private)                                       */
/*************************************************************************************/


#define Round(_x) floor(_x+0.5)

#define IntCheck(x1,y1,x2,y2,xp1,yp1,xp2,yp2,code)                                          \
  do                                                                                        \
  {                                                                                         \
     double s1,s2;                                                                          \
     s1=((x2 - x1)*(yp1 - y1) - (y2 - y1)*(xp1 - x1));                                      \
     s2=((x2 - x1)*(yp2 - y1) - (y2 - y1)*(xp2- x1));                                       \
     if((s1<0 && s2>0)||(s1>0 && s2<0)) code=POL_INTERSECT;                                 \
     else if(s1<0 && s2<0) code=POL_NOT;                                                    \
     else if(s1>0 && s2>0) code=POL_NOT;                                                    \
     else if(!s1 && s2) code=POL_VERTEX1;                                                   \
     else if(!s2 && s1) code=POL_VERTEX2;                                                   \
     else if(!s1 && !s2) code=POL_FACE;                                                     \
  }while(0)

#define Side(_x1, _y1, _x2, _y2, _ax, _ay) ((_x2 - _x1)*(_ay - _y1) - (_y2 - _y1)*(_ax - _x1))                                   


#define NLN_LeftSideUp(_cx,_cy)																															\
		 ( ( ( Dy )*( _cx - Px ) - ( _cy - Py )*( Dx ) ) >0 )? 1 : 0										

#define NLN_LeftSideDown(_cx,_cy)																													  \
		 ( ( ( Dy )*( _cx - Px ) - ( _cy - Py )*( Dx ) ) <0 )? 1 : 0																			

#define NLN_West_Intersect(_ix,_iy)																													\
  do																																												\
	{																																													\
		_iy = ( Dy / Dx )*( xmin - Px ) + Py;																										\
		_iy = Round(_iy);																																				\
		_ix = xmin;																																							\
																																														\
  }while(0)

#define NLN_East_Intersect(_ix,_iy)																													\
	do																																												\
	{																																													\
		_iy = ( Dy / Dx )*( xmax - Px ) + Py;																										\
		_iy = Round(_iy);																																				\
		_ix = xmax;																																							\
																																														\
	}while(0)

#define NLN_North_Intersect(_ix,_iy)																												\
	do																																												\
	{																																													\
		_ix = ( Dx / Dy )*( ymax - Py ) + Px;																										\
		_ix = Round(_ix);																																				\
		_iy = ymax;																																							\
																																														\
	}while(0)

#define NLN_South_Intersect(_ix,_iy)																												\
	do																																												\
	{																																													\
		_ix = ( Dx / Dy )*( ymin - Py ) + Px;																										\
		_ix = Round(_ix);																																				\
		_iy = ymin;																																							\
																																														\
  }while(0)

enum {
	NLN_LOWER_LEFT,
	NLN_UPPER_LEFT,
	NLN_UPPER_RIGHT,
	NLN_LOWER_RIGHT,
	NLN_LEFT,
	NLN_RIGHT,
	NLN_UP,
	NLN_DOWN,
	NLN_MID
};

enum {
  NLN_INVISIBLE,
	NLN_VISIBLE
};

enum {
  POL_INTERSECT,
  POL_VERTEX1,
  POL_VERTEX2,
  POL_FACE,
  POL_NOT
};  

enum{
  SLOPE_X,
  SLOPE_Y
};

enum {
  SIDE_ONE,
  SIDE_TWO
};

static void sort(int* stx, int* sty ,int left, int right,int slope)
{ 
  int i,j;                                     
  int a;                                       
  if (slope==SLOPE_X)
  {  
    i = left;                                    
    j = right;                                   
    a = stx[(left + right)/2];                    
    do                                           
    {                                          
      while (stx[i] < a && i < right) i++;      
      while (a < stx[j] && j > left)  j--;      
      if (i<=j)                                
    	  {                                        
    	    int b = stx[i];
          int by= sty[i];
    	    stx[i] = stx[j];
          sty[i] = sty[j];
          stx[j] = b;                             
    	    sty[j] = by;
          i++;j--;                               
    	  }                                        
    } while (i<=j);                            
    if (left < j)  sort(stx, sty, left, j, slope);            
    if (i < right) sort(stx, sty, i,right,slope);           
  } 
  else
  {  
    i = left;                                    
    j = right;                                   
    a = sty[(left + right)/2];                    
    do                                           
    {                                          
      while (sty[i] < a && i < right) i++;      
      while (a < sty[j] && j > left)  j--;      
      if (i<=j)                                
    	  {                                        
    	    int b = stx[i];
          int by= sty[i];
    	    stx[i] = stx[j];
          sty[i] = sty[j];
          stx[j] = b;                             
    	    sty[j] = by;
          i++;j--;                               
    	  }                                        
    } while (i<=j);                            
    if (left < j)  sort(stx, sty, left, j, slope);            
    if (i < right) sort(stx, sty, i, right, slope);           
  } 
}

static int NLNSideClip(int *_x1,int *_y1,int *_x2,int *_y2, int CornerCode)
{
	
	int visible;	 
 	int xmin = active_sim->cnv->clip_xmin;
	int xmax = active_sim->cnv->clip_xmax;
	int ymin = active_sim->cnv->clip_ymin;
	int ymax = active_sim->cnv->clip_ymax;

  double Px=(double)*_x1;
	double Qx=(double)*_x2;
	double Py=(double)*_y1;
	double Qy=(double)*_y2;

	double Dx=Qx-Px;
	double Dy=Qy-Py;

	
	switch ( CornerCode )
	{

	/*------------------------* Regiao lateral esquerda *----------------------*/

	case NLN_LEFT:
		
		if(Qx<xmin)	visible=0;														/* r0 */
		else if(Qy<=ymax)
		{
		
			if(Qy>=ymin)
			{
				visible=1;																		/* r1 */
				NLN_West_Intersect(Px,Py);
				if(Qx>xmax)																		/* r2 */
					NLN_East_Intersect(Qx,Qy);
			}
			else if(NLN_LeftSideDown(xmin,ymin)) visible=0;		/* r3 */
		    else if(Qx<=xmax) 
			{
				visible=1;																	/* r4 */
				NLN_West_Intersect(Px,Py);
				NLN_South_Intersect(Qx,Qy);
			}
			else /*if(Qx>xmax) */
			{

				if(NLN_LeftSideDown(xmax,ymin))
				{
					visible=1;																	/* r5 */
					NLN_West_Intersect(Px,Py);
					NLN_South_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																	/* r6 */
					NLN_West_Intersect(Px,Py);
					NLN_East_Intersect(Qx,Qy);
				}
			}
		}
		else
		{
            if(NLN_LeftSideUp(xmin,ymax)) visible=0;		/* r7 */
            else if(Qx<xmax) 
		    {
  	    		visible=1;																/* r8 */
  		    	NLN_West_Intersect(Px,Py);
		    	NLN_North_Intersect(Qx,Qy);								
		    }
			
			else /*if(Qx>xmax)*/
			{
				if(NLN_LeftSideUp(xmax,ymax))
				{
					visible=1;																/* r9 */
					NLN_West_Intersect(Px,Py);
					NLN_North_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																/* r10 */
					NLN_West_Intersect(Px,Py);
					NLN_East_Intersect(Qx,Qy);
				}
			}
		}	
		break;

	/*------------------------* Regiao lateral superior *----------------------*/

	case NLN_UP:
		
		if(Qy>ymax)	visible=0;														/* r0 */
		else if(Qx<=xmax)
		{
		
			if(Qx>=xmin)
			{
				visible=1;																		/* r1 */
				NLN_North_Intersect(Px,Py);
				if(Qy<ymin)																		/* r2 */
					NLN_South_Intersect(Qx,Qy);
			}
			else if(NLN_LeftSideDown(xmin,ymax)) visible=0;		/* r3 */
			else if(Qy>=ymin) 
			{
			  visible=1;																	/* r4 */
			  NLN_North_Intersect(Px,Py);
			  NLN_West_Intersect(Qx,Qy);
            }
			else 
			{
				if(NLN_LeftSideDown(xmin,ymin))
				{
					visible=1;																	/* r5 */
					NLN_North_Intersect(Px,Py);
					NLN_West_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																	/* r6 */
					NLN_North_Intersect(Px,Py);
					NLN_South_Intersect(Qx,Qy);
				}
			}
		}
		else
		{
			if(!NLN_LeftSideDown(xmax,ymax)) visible=0;	/* r7 */
			else if(Qy>ymin) 
			{
				visible=1;																/* r8 */
				NLN_North_Intersect(Px,Py);
				NLN_East_Intersect(Qx,Qy);								
	
			}
			else 
			{
				if(!NLN_LeftSideDown(xmax,ymin))
				{
					visible=1;																/* r9 */
					NLN_North_Intersect(Px,Py);
					NLN_East_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																/* r10 */
					NLN_North_Intersect(Px,Py);
					NLN_South_Intersect(Qx,Qy);
				}
			}
		}
		break;

	/*------------------------* Regiao lateral direita *----------------------*/

	case NLN_RIGHT:
		
		if(Qx>xmax)	visible=0;														/* r0 */
		else if(Qy>=ymin)
		{
		
			if(Qy<=ymax)
			{
				visible=1;																		/* r1 */
				NLN_East_Intersect(Px,Py);
				if(Qx<xmin)																		/* r2 */
					NLN_West_Intersect(Qx,Qy);
			}
			else if(!NLN_LeftSideUp(xmax,ymax))  visible=0;		/* r3 */
			else if(Qx>=xmin) 
			{
			  visible=1;																	/* r4 */
			  NLN_East_Intersect(Px,Py);
			  NLN_North_Intersect(Qx,Qy);
			}
			else 
			{
				if(!NLN_LeftSideUp(xmin,ymax))
				{
					visible=1;																	/* r5 */
					NLN_East_Intersect(Px,Py);
					NLN_North_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																	/* r6 */
					NLN_East_Intersect(Px,Py);
					NLN_West_Intersect(Qx,Qy);
				}
			}
		}
		else
		{
			if(!NLN_LeftSideDown(xmax,ymin)) visible=0;	/* r7 */
			else if(Qx>xmin) 
			{
				visible=1;																/* r8 */
				NLN_East_Intersect(Px,Py);
				NLN_South_Intersect(Qx,Qy);								
				
			}
			else 
			{
				if(!NLN_LeftSideDown(xmin,ymin))
				{
					visible=1;																/* r9 */
					NLN_East_Intersect(Px,Py);
					NLN_South_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																/* r10 */
					NLN_East_Intersect(Px,Py);
					NLN_West_Intersect(Qx,Qy);
				}
			}
		}
		break;

	/*------------------------* Regiao lateral inferior *----------------------*/

	case NLN_DOWN:
		
		if(Qy<ymin)	visible=0;														/* r0 */
		else if(Qx>=xmin)
		{
		
			if(Qx<=xmax)
			{
				visible=1;																		/* r1 */
				NLN_South_Intersect(Px,Py);
				if(Qy>ymax)																		/* r2 */
					NLN_North_Intersect(Qx,Qy);
			}
			else if(!NLN_LeftSideUp(xmax,ymin)) visible=0;		  /* r3 */
			else if(Qy<=ymax) 
			{
				visible=1;																	/* r4 */
				NLN_South_Intersect(Px,Py);
				NLN_East_Intersect(Qx,Qy);
			
			}
			else 
			{
				if(!NLN_LeftSideUp(xmin,ymin))
				{
					visible=1;																	/* r5 */
					NLN_South_Intersect(Px,Py);
					NLN_East_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																	/* r6 */
					NLN_South_Intersect(Px,Py);
					NLN_North_Intersect(Qx,Qy);
				}
			}
		}
		else
		{
			if(NLN_LeftSideUp(xmin,ymin))   visible=0;	/* r7 */
			else if(Qy<ymax) 
			{
				visible=1;																/* r8 */
				NLN_South_Intersect(Px,Py);
				NLN_West_Intersect(Qx,Qy);								
		
			}
			else 
			{
				if(NLN_LeftSideUp(xmin,ymax))
				{
					visible=1;																/* r9 */
					NLN_South_Intersect(Px,Py);
					NLN_West_Intersect(Qx,Qy);
				}
				else
				{
					visible=1;																/* r10 */
					NLN_South_Intersect(Px,Py);
					NLN_North_Intersect(Qx,Qy);
				}
			}
		}
	}

	*_x1=(int)Px;
	*_x2=(int)Qx;
	*_y1=(int)Py;
	*_y2=(int)Qy;
	
	return (visible)? NLN_VISIBLE : NLN_INVISIBLE;
}

static int NLNCornerClip(int *_x1,int *_y1,int *_x2,int *_y2, int CornerCode)
{

	int visible;	 
 	int xmin = active_sim->cnv->clip_xmin;
	int xmax = active_sim->cnv->clip_xmax;
	int ymin = active_sim->cnv->clip_ymin;
	int ymax = active_sim->cnv->clip_ymax;

  double Px=(double)*_x1;
	double Qx=(double)*_x2;
	double Py=(double)*_y1;
	double Qy=(double)*_y2;


	double Dx=Qx-Px;
	double Dy=Qy-Py;


	switch ( CornerCode )
	{
	/*
	------------------------* Regiao inferior esquerda *----------------------
	*/
	case NLN_LOWER_LEFT:
																									 
		if (Qy < ymin) visible = 0;						       					 					/* r0 */
		else if (Qx < xmin) visible =0;							      		 					/* r1 */
		else if (NLN_LeftSideUp(xmin,ymin))
		{
			if (Qy<=ymax)																	       
			{
				visible = 1;																								/* r2 */
				NLN_West_Intersect(Px,Py);
				
				if(Qx>xmax) 																       
				{																														/* r3 */
					NLN_East_Intersect(Qx,Qy);
				}
			}
			else if (NLN_LeftSideUp(xmin,ymax)) visible=0;	       				/* r4 */
			else if(Qx<xmax) 																	   
			{
				visible=1;
				NLN_West_Intersect(Px,Py);			          									/* r5 */
				NLN_North_Intersect(Qx,Qy);
			
			}
			else if(NLN_LeftSideUp(xmax,ymax))										 
			{
				visible=1;																									/* r6 */
				NLN_West_Intersect(Px,Py);
				NLN_North_Intersect(Qx,Qy);
			
			}
			else
			{
				visible=1;
				NLN_West_Intersect(Px,Py);	          											/* r7 */ 
				NLN_East_Intersect(Qx,Qy);
			}
			
		}
		else
		{
			if (Qx<=xmax)																	       
			{
				visible = 1;																								/* r2 */
				NLN_South_Intersect(Px,Py);
				
				if(Qy>ymax) 																       
				{																														/* r3 */
					NLN_North_Intersect(Qx,Qy);
					
				}
			}
			else if (!NLN_LeftSideUp(xmax,ymin)) visible = 0;      				/* r4 */
			else if(Qy<ymax) 																	   
			{
				visible = 1;
				NLN_South_Intersect(Px,Py);			          									/* r5 */
				NLN_East_Intersect(Qx,Qy);
			
			}
			else if(!NLN_LeftSideUp(xmax,ymax))										 
			{
				visible = 1;		  																					
				NLN_South_Intersect(Px,Py);		          										/* r6 */
				NLN_East_Intersect(Qx,Qy);
			
			}
			else
			{
				visible=1;
				NLN_South_Intersect(Px,Py);		           										/* r7 */ 
				NLN_North_Intersect(Qx,Qy);
			}
			
		}
    *_x1=(int)Px;
    *_x2=(int)Qx;
    *_y1=(int)Py;
    *_y2=(int)Qy;
	 	
		return (visible)? NLN_VISIBLE : NLN_INVISIBLE;
	
	case NLN_UPPER_LEFT:
	/*
	------------------------* Regiao superior esquerda *----------------------
	*/
	
		if (Qy > ymax) visible = 0;						       					 					/* r0 */
		else if (Qx < xmin) visible =0;							      		 					/* r1 */
		else if (!NLN_LeftSideDown(xmin,ymax))
		{
			if (Qx<=xmax)																	       
			{
				visible = 1;																								/* r2 */
				NLN_North_Intersect(Px,Py);
				
				if(Qy<ymin) 																       
				{																														/* r3 */
					NLN_South_Intersect(Qx,Qy);
				}
			}
			else if (!NLN_LeftSideDown(xmax,ymax)) visible=0;	     				/* r4 */
			else if(Qy>ymin) 																	   
			{
				visible=1;
				NLN_North_Intersect(Px,Py);			          									/* r5 */
				NLN_East_Intersect(Qx,Qy);
			}
			else if(!NLN_LeftSideDown(xmax,ymin))										 
			{
				visible=1;																									/* r6 */
				NLN_North_Intersect(Px,Py);
				NLN_East_Intersect(Qx,Qy);
			}
			else
			{
				visible=1;
				NLN_North_Intersect(Px,Py);	          											/* r7 */ 
				NLN_South_Intersect(Qx,Qy);
			}
			
		}
		else
		{
			if (Qy>=ymin)																	       
			{
				visible = 1;																								/* r2 */
				NLN_West_Intersect(Px,Py);
				
				if(Qx>xmax) 																       
				{																														/* r3 */
					NLN_East_Intersect(Qx,Qy);
					
				}
			}
			else if (NLN_LeftSideDown(xmin,ymin)) visible = 0;      			/* r4 */
			else if(Qx<xmax) 																	   
			{
				visible = 1;
				NLN_West_Intersect(Px,Py);			          									/* r5 */
				NLN_South_Intersect(Qx,Qy);
			
			}
			else if(NLN_LeftSideDown(xmax,ymin))										 
			{
				visible = 1;		  																					
				NLN_West_Intersect(Px,Py);		          										/* r6 */
				NLN_South_Intersect(Qx,Qy);
			
			}
			else
			{
				visible=1;
				NLN_West_Intersect(Px,Py);		           										/* r7 */ 
				NLN_East_Intersect(Qx,Qy);
			}
			
		}
    *_x1=(int)Px;
    *_x2=(int)Qx;
    *_y1=(int)Py;
    *_y2=(int)Qy;

	 	return (visible)? NLN_VISIBLE : NLN_INVISIBLE;

	/*
	------------------------* Regiao superior direita *----------------------
	*/
	case NLN_UPPER_RIGHT:
																									 
		if (Qy > ymax) visible = 0;						       					 					/* r0 */
		else if (Qx > xmax) visible =0;							      		 					/* r1 */
		else if (NLN_LeftSideDown(xmax,ymax))
		{
			if (Qx>=xmin)																	       
			{
				visible = 1;																								/* r2 */
				NLN_North_Intersect(Px,Py);
				
				if(Qy<ymin) 																       
				{																														/* r3 */
					NLN_South_Intersect(Qx,Qy);
				}
			}
			else if (NLN_LeftSideDown(xmin,ymax)) visible=0;	       			/* r4 */
			else if(Qy>ymin) 																	   
			{
				visible=1;
				NLN_North_Intersect(Px,Py);			          									/* r5 */
				NLN_West_Intersect(Qx,Qy);
			
			}
			else if(NLN_LeftSideDown(xmin,ymin))										 
			{
				visible=1;																									/* r6 */
				NLN_North_Intersect(Px,Py);
				NLN_West_Intersect(Qx,Qy);
			
			}
			else
			{
				visible=1;
				NLN_North_Intersect(Px,Py);	          											/* r7 */ 
				NLN_South_Intersect(Qx,Qy);
			}
			
		}
		else 
		{
			if (Qy>=ymin)																	       
			{
				visible = 1;																								/* r2 */
				NLN_East_Intersect(Px,Py);
				
				if(Qx<xmin) 																       
				{																														/* r3 */
					NLN_West_Intersect(Qx,Qy);
				}
			}
			else if (!NLN_LeftSideDown(xmax,ymin)) visible = 0;      			/* r4 */
			else if(Qx>xmin) 																	   
			{
				visible = 1;
				NLN_East_Intersect(Px,Py);			          									/* r5 */
				NLN_South_Intersect(Qx,Qy);
			
			}
			else if(!NLN_LeftSideDown(xmin,ymin))										 
			{
				visible = 1;		  																					
				NLN_East_Intersect(Px,Py);		          										/* r6 */
				NLN_South_Intersect(Qx,Qy);
			
			}
			else
			{
				visible=1;
				NLN_East_Intersect(Px,Py);		           										/* r7 */ 
				NLN_West_Intersect(Qx,Qy);
			}
			
		}
    *_x1=(int)Px;
    *_x2=(int)Qx;
    *_y1=(int)Py;
    *_y2=(int)Qy;
	 	
		return (visible)? NLN_VISIBLE : NLN_INVISIBLE;
	
	case NLN_LOWER_RIGHT:
	/*
	------------------------* Regiao inferior direita *----------------------
	*/
	
		if (Qy < ymin) visible = 0;						       					 					/* r0 */
		else if (Qx > xmax) visible = 0;							      		 				/* r1 */
		else if (!NLN_LeftSideUp(xmax,ymin))
		{
			if (Qy<=ymax)																	       
			{
				visible = 1;																								/* r2 */
				NLN_East_Intersect(Px,Py);
				
				if(Qx<xmin) 																       
				{																														/* r3 */
					NLN_West_Intersect(Qx,Qy);
				}
			}																								  
			else if (!NLN_LeftSideUp(xmax,ymax)) visible=0;	      				/* r4 */
			else if(Qx>xmin) 																 	   
			{
				visible=1;
				NLN_East_Intersect(Px,Py);			          									/* r5 */
				NLN_North_Intersect(Qx,Qy);
			
			}
			else if(!NLN_LeftSideUp(xmin,ymax))										 
			{
				visible=1;																									/* r6 */
				NLN_East_Intersect(Px,Py);
				NLN_North_Intersect(Qx,Qy);
			
			}
			else
			{
				visible=1;
				NLN_East_Intersect(Px,Py);	          											/* r7 */ 
				NLN_West_Intersect(Qx,Qy);
			}
			
		}
		else
		{
			if (Qx>=xmin)																	       
			{
				visible = 1;																								/* r2 */
				NLN_South_Intersect(Px,Py);
				
				if(Qy>ymax) 																       
				{																														/* r3 */
					NLN_North_Intersect(Qx,Qy);
					
				}
			}
			else if (NLN_LeftSideUp(xmin,ymin)) visible = 0;      			 /* r4 */
			else if(Qy<ymax) 																	   
			{
				visible = 1;
				NLN_South_Intersect(Px,Py);			          									/* r5 */
				NLN_West_Intersect(Qx,Qy);
			
			}
			else if(NLN_LeftSideUp(xmin,ymax))										 
			{
				visible = 1;		  																					
				NLN_South_Intersect(Px,Py);		          										/* r6 */
				NLN_West_Intersect(Qx,Qy);
			
			}
			else
			{
				visible = 1;
				NLN_South_Intersect(Px,Py);		           										/* r7 */ 
				NLN_North_Intersect(Qx,Qy);
			}
			
		}
    *_x1=(int)Px;
    *_x2=(int)Qx;
    *_y1=(int)Py;
    *_y2=(int)Qy;

	 	return (visible)? NLN_VISIBLE : NLN_INVISIBLE;
	}

  return 0;
}

static int NLNCenterClip(int *_x1,int *_y1,int *_x2,int *_y2)
{

	int visible;	 
 	int xmin = active_sim->cnv->clip_xmin;
	int xmax = active_sim->cnv->clip_xmax;
	int ymin = active_sim->cnv->clip_ymin;
	int ymax = active_sim->cnv->clip_ymax;

  double x1=(double)*_x1;
	double x2=(double)*_x2;
	double y1=(double)*_y1;
	double y2=(double)*_y2;

  double Dx=x2-x1;
	double Dy=y2-y1;

	if(y2<=ymax && x2<=xmax && x2>=xmin && y2>=ymin)	visible = 1;
	
	else
	{
		int temp_x, temp_y;
	
		temp_x=*_x1;
		temp_y=*_y1;
		*_x1=*_x2;
		*_y1=*_y2;
		*_x2=temp_x;
		*_y2=temp_y;

		if(x2<xmin)
		{
			if(y2<ymin)				visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_LOWER_LEFT);
			else if(y2>ymax)	visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_UPPER_LEFT);
			else							visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_LEFT);
		}
		else if(x2>xmax)
		{
			if(y2<ymin)				visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_LOWER_RIGHT);
			else if(y2>ymax)	visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_UPPER_RIGHT);
		  else 							visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_RIGHT);
		}
		else
		{
		  if(y2<ymin)				visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_DOWN);
			else						  visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_UP);
		}
	}
	return visible;
}


/*************************************************************************************/
/*                          CLIPPING (public)                                        */
/*************************************************************************************/

int cdSimClipPointInBox(int x, int y)                            
{
  if (x < active_sim->cnv->clip_xmin ||   
      x > active_sim->cnv->clip_xmax ||   
      y < active_sim->cnv->clip_ymin ||   
      y > active_sim->cnv->clip_ymax)     
    return 0;

  return 1;
}

/* clipping de ponto em poligono */
int cdSimClipPointInPoly(int px, int py)
{
  int i, cont=0;
  int nx,ny;
  int ax,ay;
  int cx,cy;
  int nnx,nny;
  int vertexSide1,vertexSide2;
  int IntersType;
  int oldcase;

  for (i=0; i < active_sim->clippoly.n - 1; i++)
  {
    cx=active_sim->clippoly.pntx[i];
    cy=active_sim->clippoly.pnty[i];
    nx=active_sim->clippoly.pntx[i+1];
    ny=active_sim->clippoly.pnty[i+1];

    if(i==active_sim->clippoly.n-2)
    {
      nnx=active_sim->clippoly.pntx[1];
      nny=active_sim->clippoly.pnty[1];
    }
    else
    {
      nnx=active_sim->clippoly.pntx[i+2];
      nny=active_sim->clippoly.pnty[i+2];
    }
    if(!i)
    {
      ax=active_sim->clippoly.pntx[active_sim->clippoly.n - 2];
      ay=active_sim->clippoly.pnty[active_sim->clippoly.n - 2];
    }
    else
    {
      ax=active_sim->clippoly.pntx[i-1];
      ay=active_sim->clippoly.pnty[i-1];
    }

    IntCheck(px, py, px-1000, py, cx, cy, nx, ny, IntersType);

    switch(IntersType)
    {
    case POL_INTERSECT:
      {
        int x;
        x = (int)Round((double)cx + (double)(( nx - cx) * (py - cy)) / (double)( ny - cy));
        if(x < px)
          cont++;
        break;
      }

    case POL_FACE:
      if(cx>nx)
      {  
        if(px>cx)
        {
          if(Side(px-1000,py,px,py,ax,ay)>0)
            vertexSide1=SIDE_ONE;
          else
            vertexSide1=SIDE_TWO;
          if(Side(px-1000,py,px,py,nnx,nny)>0)
            vertexSide2=SIDE_ONE;
          else
            vertexSide2=SIDE_TWO;
          
          if(vertexSide1!=vertexSide2)
            cont++;
      
        }
        else if(px<cx && px>nx)
        {
          if(Side(px-1000,py,px,py,ax,ay)>0)
            vertexSide1=SIDE_ONE;
          else
            vertexSide1=SIDE_TWO;
          if(Side(px-1000,py,px,py,nnx,nny)>0)
            vertexSide2=SIDE_ONE;
          else
            vertexSide2=SIDE_TWO;
          
          if(vertexSide1==vertexSide2)
            cont++;
        }

      }
      else
      {
        if(px>nx)
        {
          if(Side(px-1000,py,px,py,ax,ay)>0)
            vertexSide1=SIDE_ONE;
          else
            vertexSide1=SIDE_TWO;
          if(Side(px-1000,py,px,py,nnx,nny)>0)
            vertexSide2=SIDE_ONE;
          else
            vertexSide2=SIDE_TWO;
          
          if(vertexSide1!=vertexSide2)
            cont++;
        }
        else if(px>cx && px<nx)
        {
          if(Side(px-1000,py,px,py,ax,ay)>0)
            vertexSide1=SIDE_ONE;
          else
            vertexSide1=SIDE_TWO;
          if(Side(px-1000,py,px,py,nnx,nny)>0)
            vertexSide2=SIDE_ONE;
          else
            vertexSide2=SIDE_TWO;
          
          if(vertexSide1==vertexSide2)
            cont++;
        } 

      }

        
      break;

    case POL_VERTEX1:
      if (oldcase==POL_FACE) break;
      if(px>cx)
      {
         if(Side(px-1000,py,px,py,ax,ay)>0)
          vertexSide1=SIDE_ONE;
        else
          vertexSide1=SIDE_TWO;
        if(Side(px-1000,py,px,py,nx,ny)>0)
          vertexSide2=SIDE_ONE;
        else
          vertexSide2=SIDE_TWO;
        if(vertexSide1!=vertexSide2)
          cont++;
      }
      break;
      
    case POL_VERTEX2:
    case POL_NOT: 
      break;
    }
    oldcase=IntersType;
  }

  return (cont%2);  
}

/* 						    					Clipping de linha 
------------------* Algoritmo de Nicholl-Lee-Nicholl *---------------------------
*/
int cdSimClipLineInBox(int *_x1,int *_y1,int *_x2,int *_y2)
{

	int visible;
	
	int x1=*_x1;
	int x2=*_x2;
	int y1=*_y1;
	int y2=*_y2;

 	int xmin = active_sim->cnv->clip_xmin;
	int xmax = active_sim->cnv->clip_xmax;
	int ymin = active_sim->cnv->clip_ymin;
	int ymax = active_sim->cnv->clip_ymax;
	
	visible=0;

	if(x1<xmin)
	{
		if(y1<ymin)				visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_LOWER_LEFT);
		else if(y1>ymax)	visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_UPPER_LEFT);
		else							visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_LEFT);
	}
	else if(x1>xmax)
	{
		if(y1<ymin)				visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_LOWER_RIGHT);
		else if(y1>ymax)	visible=NLNCornerClip(_x1,_y1,_x2,_y2,NLN_UPPER_RIGHT);
	  else 							visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_RIGHT);
	}
	else
	{
	  if(y1<ymin)				visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_DOWN);
		else if(y1>ymax)  visible=NLNSideClip(_x1,_y1,_x2,_y2,NLN_UP);
		else							visible=NLNCenterClip(_x1,_y1,_x2,_y2);
	}
	
	return visible;

}

/* clipping de retangulo em retangulo */
int cdSimClipBoxInBox(int *xmin, int *xmax, int *ymin, int *ymax)
{
  if(active_sim->cnv->clip_xmin < *xmax &&
     active_sim->cnv->clip_ymin < *ymax &&
     active_sim->cnv->clip_xmax > *xmin &&
     active_sim->cnv->clip_ymax > *ymin)
	{
	  if(active_sim->cnv->clip_xmin>*xmin) *xmin = active_sim->cnv->clip_xmin;
	  if(active_sim->cnv->clip_xmax<*xmax) *xmax = active_sim->cnv->clip_xmax;
		if(active_sim->cnv->clip_ymin>*ymin) *ymin = active_sim->cnv->clip_ymin;
		if(active_sim->cnv->clip_ymax<*ymax) *ymax = active_sim->cnv->clip_ymax;

	  return 1;
  }
  else 
    return 0;
}

void cdSimClipLineInPoly(int x1,int y1,int x2,int y2)
{
  int cx,cy;           /*vertice corrente*/
  int code;            /*tipo de intersecao*/
  int px,py;           /*vertice anterior*/
  int nx,ny;           /*proximo vertice*/
  int rx,ry;           /*ponto de intersecao corrente*/
  int nnx,nny;         /*2 vertices depois do corrente*/
  int vertexSide1,vertexSide2; 
  int i=0;
  int scount=0;
  int slope;            /*octante */
  int In;
  int j;
 
  int oldcase;

  int *stx = active_sim->clippoly.sgmx;
  int *sty = active_sim->clippoly.sgmy;

  cdClip(CD_CLIPOFF);

  if(abs(x2-x1)>abs(y2-y1))
  {
    if(x1>x2)
    {
      int tempx,tempy;
      tempx=x1;
      tempy=y1;
      x1=x2;
      y1=y2;
      x2=tempx;
      y2=tempy;
    }

    slope=SLOPE_X;
  }
  else
  {
    if(y1>y2)
    {
      int tempx,tempy;
      tempx=x1;
      tempy=y1;
      x1=x2;
      y1=y2;
      x2=tempx;
      y2=tempy;		 
		}
    slope=SLOPE_Y;
  }

  for(i=0;i<active_sim->clippoly.n-1;i++)
  {
    cx=active_sim->clippoly.pntx[i];
    cy=active_sim->clippoly.pnty[i];
    nx=active_sim->clippoly.pntx[i+1];
    ny=active_sim->clippoly.pnty[i+1];
    IntCheck(x1,y1,x2,y2,cx,cy,nx,ny,code);
    switch(code)
    {
    case POL_INTERSECT:
      if(y1!=y2)
      {                                                                                         
        double a,c,b,d,t,dbx,dby;                                                               
        a = y2 - y1;                                                                             
        b = ny - cy;                                                                            
        c = x2 - x1;                                                                            
        d = nx - cx;                                                                            
        t = (b * (cx - x1) - d * (cy - y1)) / (c*b - d*a);                                      
        dbx = x1 + c * t;                                                                       
        dby = y1 + a * t;                                                                       
        rx=(int)Round(dbx);                                                                     
        ry=(int)Round(dby);                                                                     
      }
      else
      {
        rx = (int)Round((double)cx + (double)(( nx - cx) * (y1 - cy)) / (double)( ny - cy));
        ry = y1;
      }
      if(slope==SLOPE_X)
      {
        if(rx>=x1 && rx<=x2)
        {
          stx[scount]=rx;
          sty[scount++]=ry;
        }
      }
      else
      {
        if(ry>=y1 && ry<=y2)
        {
          stx[scount]=rx;
          sty[scount++]=ry;
        }
      }
      break;
    case POL_VERTEX1:
      if(oldcase==POL_FACE)
        break;

      if(slope==SLOPE_X)
      {
        if(x1<cx && x2<cx)
          break;
        else if(x1>cx && x2>cx)
          break;
      }
      else
      {
        if(y1<cy && y2<cy)
          break;
        else if(y2>cy && y2>cy)
          break;
      }

      stx[scount]=cx;
      sty[scount++]=cy;

      if(!i)
      {
        px=active_sim->clippoly.pntx[active_sim->clippoly.n-2];
        py=active_sim->clippoly.pnty[active_sim->clippoly.n-2];
      }
      else
      {
        px=active_sim->clippoly.pntx[i-1];
        py=active_sim->clippoly.pnty[i-1];
      }

      if(Side(x1,y1,x2,y2,px,py)>0)
        vertexSide1=SIDE_ONE;
      else
        vertexSide1=SIDE_TWO;
      if(Side(x1,y1,x2,y2,nx,ny)>0)
        vertexSide2=SIDE_ONE;
      else
        vertexSide2=SIDE_TWO;

      if(vertexSide1==vertexSide2)
      {
        stx[scount]=cx;
        sty[scount++]=cy;
      }
      break;
    case POL_FACE:
      if(slope==SLOPE_X)
      {
        if(cx<nx)
        {
          if(x1<cx && x2<cx)
            break;
          else if(x1>nx && x2>nx)
            break;
          else if(x1<cx && x2>cx && x2<nx)
          {
            stx[scount]=cx;
            sty[scount++]=cy;
            break;
          
          }
          else if(x1>cx && x1<nx && x2>nx)
          {
            stx[scount]=nx;
            sty[scount++]=ny;
            break;
          }
          else if(x1>cx && x2<nx)
          {
            active_sim->cnv->Line(x1,y1,x2,y2);
            cdClip(CD_CLIPPOLYGON);
            return;
          }
        }
        else
        {
          if(x1<nx && x2<nx)
            break;
          else if(x1>cx && x2>cx)
            break;
          else if(x1<nx && x2>nx && x2<cx)
          {
            stx[scount]=nx;
            sty[scount++]=ny;
            break;
          
          }
          else if(x1>nx && x1<cx && x2>cx)
          {
            stx[scount]=cx;
            sty[scount++]=cy;
            break;
          }
          else if(x1>nx && x2<cx)
          {   
            active_sim->cnv->Line(x1,y1,x2,y2);
            cdClip(CD_CLIPPOLYGON);
            return;
          }
        }
      }
      else
      {
        if(cx<nx)
        {
          if(y1<cy && y2<cy)
            break;
          else if(y2>ny && y2>ny)
            break;
          else if(y1<cy && y2>cy && y2<ny)
          {
            stx[scount]=cx;
            sty[scount++]=cy;
            break;
          }
          else if(y1>cy && y1<ny && y2>ny)
          {
            stx[scount]=nx;
            sty[scount++]=ny;
            break;
          }
          else if(y1>cy && y2<ny)
          {
            active_sim->cnv->Line(x1,y1,x2,y2);
            cdClip(CD_CLIPPOLYGON);
            return;
          }
        }
        else
        {
          if(y1<ny && y2<ny)
            break;
          else if(y2>cy && y2>cy)
            break;
          else if(y1<ny && y2>ny && y2<cy)
          {
            stx[scount]=nx;
            sty[scount++]=ny;
            break;
          }
          else if(y1>ny && y1<cy && y2>cy)
          {
            stx[scount]=cx;
            sty[scount++]=cy;
            break;
          }
          else if(y1>ny && y2<cy)
          {
            active_sim->cnv->Line(x1,y1,x2,y2);
            cdClip(CD_CLIPPOLYGON);
            return;
          }
        }
      }

      if(!i)
      {
        px=active_sim->clippoly.pntx[active_sim->clippoly.n-1];
        py=active_sim->clippoly.pnty[active_sim->clippoly.n-1];
      }
      else
      {
        px=active_sim->clippoly.pntx[i-1];
        py=active_sim->clippoly.pnty[i-1];
      }

      if(i==active_sim->clippoly.n-2)
      {
        nnx=active_sim->clippoly.pntx[1];
        nny=active_sim->clippoly.pnty[1];

      }
      else
      {
        nnx=active_sim->clippoly.pntx[i+2];
        nny=active_sim->clippoly.pnty[i+2];
      }

      if(Side(x1,y1,x2,y2,px,py)>0)
        vertexSide1=SIDE_ONE;
      else
        vertexSide1=SIDE_TWO;

      if(Side(x1,y1,x2,y2,nnx,nny)>0)
        vertexSide2=SIDE_ONE;
      else
        vertexSide2=SIDE_TWO;
         
      stx[scount]=cx;
      sty[scount++]=cy;
      stx[scount]=nx;
      sty[scount++]=ny;
      
      if(vertexSide1!=vertexSide2)
      {
        stx[scount]=nx;
        sty[scount++]=ny;
      }
      break;

    case POL_NOT:
    case POL_VERTEX2:
      break;
    }
    oldcase=code;
  }
 
  stx[scount]=x1;
  sty[scount++]=y1;
  stx[scount]=x2;
  sty[scount]=y2;

  sort(stx, sty, 0, scount, slope);
  
  In = cdSimClipPointInPoly(stx[0],sty[0]);
  
  for(j=(1-In);j+1<=scount;j+=2)
  {
    if(active_sim->context == CLIP_REGION)
      simDoFillLine(stx[j], sty[j], stx[j+1], sty[j+1]);
    else 
      active_sim->cnv->Line(stx[j],sty[j],stx[j+1],sty[j+1]);
  }

  cdClip(CD_CLIPPOLYGON);
}

int cdSimClipTextInBox(int x, int y,char * text)
{
  int delta_x,delta_y;
  int xmin,xmax;
  int ymin,ymax;
  int textlength;
  int alignment = cdTextAlignment(CD_QUERY);
  
  cdTextSize(text,&delta_x,&delta_y);
  delta_x-=1;
  delta_y-=1;

  switch(alignment)
  {
  case CD_NORTH:
    xmin = x - (int)(delta_x/2);
    ymin = y - (int)(delta_y);
    xmax = xmin + delta_x;
    ymax = y;
    break;
  case CD_WEST:
    xmin = x;
    ymin = y - (int)(delta_y/2);
    xmax = x + delta_x;
    ymax = ymin + delta_y;
    break;
  case CD_SOUTH:
    xmin = x - (int)(delta_x/2);
    ymin = y;
    xmax = xmin + delta_x;
    ymax = y + delta_y;
    break;
  case CD_EAST:
    xmin = x - delta_x;
    ymin = y - (int)(delta_y/2);
    xmax = x;
    ymax = ymin + delta_y;
    break;
  case CD_CENTER:
    xmin = x - (int)(delta_x/2);
    ymin = y - (int)(delta_y/2);
    xmax = xmin + delta_x;
    ymax = ymin + delta_y;
    break;
  case CD_NORTH_EAST:
    xmin = x - delta_x;
    ymin = y - delta_y;
    xmax = x;
    ymax = y;
    break;
  case CD_NORTH_WEST:
    xmin = x;
    ymin = y - delta_y;
    xmax = x + delta_x;
    ymax = y;
    break;
  case CD_SOUTH_EAST:
    xmin = x - delta_x;
    ymin = y;
    xmax = x;
    ymax = y + delta_y;
    break;
  case CD_SOUTH_WEST:
    xmin = x; 
    ymin = y;
    xmax = x + delta_x;
    ymax = y + delta_y;
    break;
  case CD_BASE_CENTER:
    {
      int ascent, height, baseline;
      cdFontDim(NULL, &height, &ascent, NULL);
      baseline = height - ascent;
      
      if (active_sim->cnv->invert_yaxis)
        baseline *= -1;

      xmin = x - (int)(delta_x/2);
      ymin = y - baseline;
      xmax = xmin + delta_x;
      ymax = ymin + delta_y;
    }
    break;
  case CD_BASE_RIGHT:
    {
      int ascent, height, baseline;
      cdFontDim(NULL, &height, &ascent, NULL);
      baseline = height - ascent;
      
      if (active_sim->cnv->invert_yaxis)
        baseline *= -1;

      xmin = x - delta_x;
      ymin = y - baseline;
      xmax = xmin + delta_x;
      ymax = ymin + delta_y;
    }
    break;
  case CD_BASE_LEFT:
    {
      int descent;
      cdFontDim(0,0,0,&descent);
      xmin = x;
      ymin = y - descent;
      xmax = x + delta_x;
      ymax = ymin + delta_y;  
    }
    break;
  }

  if(active_sim->cnv->clip_xmin>xmax)
    return 0;
  else if(active_sim->cnv->clip_xmax<xmin)
    return 0;
  else if(active_sim->cnv->clip_ymin>ymin)
    return 0;
  else if(active_sim->cnv->clip_ymax<ymax)
    return 0;
  else if(active_sim->cnv->clip_xmin<=xmin &&
          active_sim->cnv->clip_ymin<=ymin &&
          active_sim->cnv->clip_xmax>=xmax &&
          active_sim->cnv->clip_ymax>=ymax )
    return 1;
  else 
  {
    int StatusCode;
    int estchar;
    int xclip;
    int width;
  
    cdClip(CD_CLIPOFF);
  
    if(active_sim->cnv->clip_xmin>xmin) 
    {
      textlength = strlen(text);
      estchar = (int) textlength * (active_sim->cnv->clip_xmin - xmin) / (xmax - xmin);
      
      cdTextSize(text+estchar,&width,0);
      xclip = xmax - width + 1;
      
      if(xclip<active_sim->cnv->clip_xmin) 
        StatusCode=1;
      else if(xclip>active_sim->cnv->clip_xmin)
        StatusCode=-1;
      else
        StatusCode=0;

      do 
      {
        switch(StatusCode)
        {
        case 0:
          if(xmax<=active_sim->cnv->clip_xmax)
          {
            cdTextAlignment(CD_SOUTH_WEST);
            active_sim->cnv->Text(xclip,ymin,text+estchar); 
          }
          else
          {
            cdTextAlignment(CD_SOUTH_WEST);
            cdSimClipTextInBox(xclip,ymin,text+estchar); 
          }
          cdClip(CD_CLIPAREA);
          return 0;
        case 1:
          estchar++;
          cdTextSize(text+estchar,&width,0);
          xclip = xmax - width + 1;
          if(xclip>=active_sim->cnv->clip_xmin)
            StatusCode=0;
          break;
        case -1:
          estchar--;
          cdTextSize(text+estchar,&width,0);
          xclip = xmax - width + 1;
          if(xclip<active_sim->cnv->clip_xmin)
          {
            estchar++;
            cdTextSize(text+estchar,&width,0);
            xclip = xmax - width + 1;
            StatusCode=0;
          }
          else if(xclip==active_sim->cnv->clip_xmin)
            StatusCode=0;
          break;
        }
      }while(1);
    }
    else if(active_sim->cnv->clip_xmax<xmax) 
    {
      
      textlength = strlen(text);
      estchar = (int) textlength * (active_sim->cnv->clip_xmax - xmin) / (xmax - xmin);
      
      cdTextSize(text+estchar,&width,0);
      xclip = xmax - width + 1;
      
      if(xclip>active_sim->cnv->clip_xmax) 
        StatusCode=1;
      else if(xclip<active_sim->cnv->clip_xmax)
        StatusCode=-1;
      else
        StatusCode=0;
      do 
      {
        switch(StatusCode)
        {
        case 0:
          text[estchar]='\0';
          cdTextAlignment(CD_SOUTH_EAST);
          active_sim->cnv->Text(xclip,ymin,text);
          cdClip(CD_CLIPAREA);
          return 0;
        case 1:
          estchar--;
          cdTextSize(text+estchar,&width,0);
          xclip = xmax - width + 1;
          if(xclip>=active_sim->cnv->clip_xmin)
            StatusCode=0;
          break;
        case -1:
          estchar++;
          cdTextSize(text+estchar,&width,0);
          xclip = xmax - width + 1;
          if(xclip>active_sim->cnv->clip_xmax)
          {
            estchar--;
            cdTextSize(text+estchar,&width,0);
            xclip = xmax - width + 1;
            StatusCode=0;
          }
          else if(xclip==active_sim->cnv->clip_xmin)
            StatusCode=0;
          break;
        }
      }while(1);
    }		

    cdClip(CD_CLIPAREA);
  }	
  
  return 0;
}
