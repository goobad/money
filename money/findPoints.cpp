#include "findPoints.h"
#include <iostream>
using namespace std;
int _height;//图像高度
int _startY[2];//左右边采样开始的y坐标,_startY[0]为左边的
int _numLR[2];//左右边采样点数，_numLR[0]为左边的
int _pointPosition[2][SamplePoints];//上下边界点
int _pointPositionLR[2][200];//左右边界点
int UpValidPoint[2],DownValidPoint[2];//上下边有效点
int pointLocation[8];
int pointLocation1[8];

extern struct boardConfig configSet;

void initPointPosition ()
{
	int i;
    for (i = 0; i < SamplePoints; i++)
    {
        _pointPosition[0][i] = StartX + (i << SampleStepLogX);//横坐标
    }
}

/*
 *  每一列找其边界点
 *  @param [in] imageStart 列开始坐标
 *  @param [in] position 方向1：上边 -1：下边  
 *  @return 边界Y坐标
 */
 int ColumnFindPoint(Byte * start, int position)
 {
	int i,j,pointY;
	int bigSearchNum = _height/BigSearchY - 1, smallSearchNum = 2*BigSearchY/SmallSearchY, searchNum = 2*SmallSearchY;//大小及单行搜索的次数
	int bigBackPoints = position*WidDpi*(2*BigSearchY - SmallSearchY);//大搜索回跳点数
	int smallBackPoints = position*WidDpi*(2*SmallSearchY - 1);//小搜索回跳点数
	int bigStep = position*BigSearchY*WidDpi,smallStep = position*SmallSearchY*WidDpi,step = position*WidDpi;//大、小及单行步长
	Byte th = configSet.ValueThreshold;
	Byte *pr = start;
	//Byte th = 50;
	//头BigSearchY行特殊处理
	pr += bigStep;
	if(*pr > th)
	{
		pr = start;
		for(pointY = 0;pointY < BigSearchY; pointY++)
		{
			if(*pr > th)
			{
				if(position > 0)
					return pointY;
				else
					return _height - pointY; 
			}
			pr += step;
		}
	}
	
	//剩余行	
	for(i = 1;i < bigSearchNum;i++)
	{
		pr += bigStep;
		if(*pr > th)
		{
			pr -= bigBackPoints;
			for(j = 0;j < smallSearchNum; j++)
			{
				if(*pr > th)
				{
					pr -= smallBackPoints;
					for(pointY = 0;pointY < searchNum;pointY++)
					{
						if(*pr > th)
						{
							if(position > 0)
								return (i-1)*BigSearchY + (j-1)*SmallSearchY + pointY + 1;
							else
								return _height - (i-1)*BigSearchY - (j-1)*SmallSearchY - pointY - 1; 
						}	
						pr += step;
					}
				}
				pr += smallStep;				
			}
		}
	}
    return -1;		
 }
 
/*
 *  每一行找其边界点
 *  @param [in] imageStart 行开始坐标
 *  @param [in] position 方向1：左边 -1：右边  
 *  @return 边界X坐标
 */
 int RowFindPoint(Byte * start, int position)
 {
	int i,j,pointX;
	int bigSearchNum = (WidDpi - AbandanArrange)/BigSearchX - 3, smallSearchNum = 2*BigSearchX/SmallSearchX, searchNum = 2*SmallSearchX;//大小及单列搜索的次数
	int bigBackPoints = position*(2*BigSearchX - SmallSearchX);//大搜索回跳点数
	int smallBackPoints = position*(2*SmallSearchX - 1);//小搜索回跳点数
	int bigStep = position*BigSearchX,smallStep = position*SmallSearchX,step = position;//大、小及单行步长
	int dotStep = position*DotSize;//考虑积灰的大小
	Byte th = configSet.ValueThreshold, *pr = start;
	
	//头BigSearchX列特殊处理
	pr += bigStep;
	if(*pr > th)
	{
		pr = start;
		for(pointX = 0;pointX < BigSearchX; pointX++)
		{
			if(*pr > th && *(pr + dotStep) > th)
			{
				if(position > 0)
					return pointX;
				else
					return WidDpi - AbandanArrange - 1 - pointX; 
			}
			pr += step;
		}
	}
	
	//剩余列	
	for(i = 1;i < bigSearchNum;i++)
	{
		pr += bigStep;
		if(*pr > th && *(pr + dotStep) > th)
		{
			pr -= bigBackPoints;
			for(j = 0;j < smallSearchNum; j++)
			{
				if(*pr > th && *(pr + dotStep) > th)
				{
					pr -= smallBackPoints;
					for(pointX = 0;pointX < searchNum;pointX++)
					{
						if(*pr > th && *(pr + dotStep) > th)
						{
							if(position > 0)
								return (i-1)*BigSearchX + (j-1)*SmallSearchX + pointX + 1;
							else
								return WidDpi - AbandanArrange - 1 - (i-1)*BigSearchX - (j-1)*SmallSearchX - pointX - 1; 
						}	
						pr += step;
					}
				}
				pr += smallStep;				
			}
		}
	}
    return -1;		
 }

 /*
 *  找上边
 *  @param [in] imageStart 图像开始地址
 *  @param [in] imageEnd 图像结束地址 
 *  @param [out] line 边线的斜率和切距的数组
 *  @return 无
 */
int FindUpLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, n, validN = 0, gradientTab,smallGradient, bigGradient,  fixPoints = 0;
    Byte *currentStart;
    int pGradient[SamplePoints], pX[SamplePoints], pY[SamplePoints];

    currentStart = imageStart + StartX;//开始采样的地址
    gradientTab = _height<<1;
    //采样找边界点
    for (i = 0; i < SamplePoints; i++)
    {
	
		//每列都进行独立的跳跃找边
		_pointPosition[1][i] = ColumnFindPoint(currentStart + (i << SampleStepLogX), 1);
		
		//计算采样点间梯度用于剔除异常点
        if (i > 1)
        {
            //间隔点间纵坐标的差
            if((_pointPosition[1][i - 2]!=-1)&&(_pointPosition[1][i]!=-1))
            {
                pGradient[i - 2] = _pointPosition[1][i - 2] - _pointPosition[1][i];
                pX[validN++] = pGradient[i - 2];
            }
            else
                pGradient[i - 2] = gradientTab;
        }
    }
    if(validN<10)
        return 1;//采样成功的点数过少
    SortArray(pX, validN);//排序
    smallGradient = pX[validN>> 2];//四分位数
    bigGradient = pX[validN - (validN>> 2)];
    //筛查出用于拟合的点
	n = SamplePoints - 2;
    for (i = 0; i < n; i++)
    {
        if ((pGradient[i] <= bigGradient) && (pGradient[i] >= smallGradient))
        {
            pX[fixPoints] = _pointPosition[0][i];//用于拟合的横坐标
            pY[fixPoints++] = _pointPosition[1][i];//用于拟合的纵坐标
        }
    }

	for (int i = 0; i < 60; i++)
	{
		cout << pX[i] << " ";
	}
	cout << endl;
	for (int i = 0; i < 60; i++)
	{
		cout << pY[i] << " ";
	}
	cout << endl;

    fixLine(pX, pY, fixPoints, line);//拟合直线
	UpValidPoint[0] = pX[0];//记录第一个有效点
	UpValidPoint[1] = pY[0];
    return 0;
}
/*
 *  找下边
 *  @param [in] imageStart 图像开始地址
 *  @param [in] imageEnd 图像结束地址 
 *  @param [out] line 边线的斜率和切距的数组
 *  @return 无
 */
int FindDownLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, n, validN = 0, gradientTab, smallGradient, bigGradient, fixPoints = 0;
    Byte *currentStart;
    int pGradient[SamplePoints], pX[SamplePoints], pY[SamplePoints];

    currentStart = imageEnd - WidDpi + StartX;//开始采样的地址
    gradientTab = _height<<1;
    //采样找边界点
    for (i = 0; i < SamplePoints; i++)
    {

		//每列都进行独立的跳跃找边
		_pointPosition[1][i] = ColumnFindPoint(currentStart + (i << SampleStepLogX), -1);
		
        if (i > 1)
        {
            //间隔点间纵坐标的差
            if((_pointPosition[1][i - 2]!=-1)&&(_pointPosition[1][i]!=-1))
            {
                pGradient[i - 2] = _pointPosition[1][i - 2] - _pointPosition[1][i];
                pX[validN++] = pGradient[i - 2];
            }
            else
                pGradient[i - 2] = gradientTab;
        }
    }
    if(validN<10)
        return 1;
    SortArray(pX, validN);//排序
    smallGradient = pX[validN>> 2];//四分位数
    bigGradient = pX[validN - (validN>> 2)];
    //筛查出用于拟合的点
	n = SamplePoints - 2;
    for (i = 0; i < n; i++)
    {
        if ((pGradient[i] <= bigGradient) && (pGradient[i] >= smallGradient))
        {
            pX[fixPoints] = _pointPosition[0][i];//用于拟合的横坐标
            pY[fixPoints++] = _pointPosition[1][i];//用于拟合的纵坐标
        }
    }
    fixLine(pX, pY, fixPoints, line);//拟合直线
	DownValidPoint[0] = pX[0];//记录第一个有效点
	DownValidPoint[1] = pY[0];
    return 0;
}
/*
 *  找钞票上下起始边界
 *  @param [in] imageStart 图像开始地址
 *  @param [in] orient 钞票上边倾斜方向（斜率大于0为1小于0为0）
 *  @param [in] small 上边与图像左右边相交的最低点
 *  @param [in] big 下边与图像左右边相交的最高点
 *  @return 无
 */
void DefineSearchLR(Byte * imageStart, char orient,int small, int big,int flag)
{
	int i,j,horizonStep, doubleStep,verticStep, topSearchN,downSearchN,horizonSearchN;
	int topEdge, downEdge;
	Byte * currentT, *currentD, *pp;
	topSearchN = small/5;		  //间隔五行搜索找上界的最大循环次数
	downSearchN = (_height-big)/5;//间隔五行搜索找下界的最大循环次数
	horizonSearchN = (WidDpi - AbandanArrange)/10 - 2;	  //间隔10列搜索找边界的最大循环次数
	verticStep = 5*WidDpi;		  //纵向步长
	//初始化上下界
	topEdge = 0;		//上界
	downEdge= _height;	//界
	
	if(orient)
	{
		currentT = imageStart + small*WidDpi;		//找辖绲乃阉鞯刂?		
		currentD = imageStart + (big+1)*WidDpi - 1 - AbandanArrange;	//找下界的搜索地址
		horizonStep = 10;//水平步长
		doubleStep = horizonStep<<1;//两倍水平步长
	}
	else
	{
		currentT = imageStart + (small+1)*WidDpi - 1 - AbandanArrange;
		currentD = imageStart + big*WidDpi;
		horizonStep = -10;
		doubleStep = horizonStep<<1;
	}
	//找上起始边界
	for(i = 0; i<topSearchN;i++ )
	{
		pp = currentT;
		for(j=0;j<horizonSearchN;j++ )
		{
			//如果当前点及平移步长两次中有一个点都不是背景则换行继续搜索
			if((*pp > configSet.ValueThreshold)&&((*(pp+horizonStep) >
				configSet.ValueThreshold)||(*(pp+doubleStep) > configSet.ValueThreshold)))
				break;
			pp += horizonStep;
		}
		//如果这一行是背景，找到上起始边界，记录下Y坐标
		if(j == horizonSearchN)
		{
			topEdge = small - (i+1)*5;//记录下下界
			break;
		}
		currentT -=verticStep;
	}
	//找下起始边界
	for(i = 0; i<downSearchN;i++ )
	{
		pp = currentD;
		for(j=0;j<horizonSearchN;j++ )
		{
			//如果当前点及平移步长两次中有一个点都不是背景则换行继续搜索
			if((*pp > configSet.ValueThreshold)&&((*(pp-horizonStep) > 
				configSet.ValueThreshold)||(*(pp-doubleStep) > configSet.ValueThreshold)))
				break;
			pp -= horizonStep;
		}
		//如果这一行是背景
		if(j == horizonSearchN)
		{
			downEdge = big + (i-1)*5;//记录下下界
			break;
		}
		currentD +=verticStep;
	}
	if(orient)//钞票上边斜率大于0
	{
		//找左边拟合点时的搜索起始纵坐标和搜索次数
		_startY[0] = topEdge + ((big - topEdge) >> 2);		
	    _numLR[0] = ((big - topEdge) >> 1) >> SampleStepLogY;
		
		//找右边拟合点时的搜索起始纵坐标和搜鞔问?
		if(flag)
		{
			_startY[1] = small + ((downEdge - small) >> 1);		
		    _numLR[1] = ((downEdge - small) >> 2) >> SampleStepLogY;
		}
		else
		{
			_startY[1] = small + ((downEdge - small) >> 2);		
		    _numLR[1] = ((downEdge - small) >> 1) >> SampleStepLogY;
		}
	}
	else
	{
		//找右边拟合点时的搜索起始纵坐标和搜索次数
		if(flag)
		{
			_startY[1] = topEdge + ((big - topEdge) >> 2);		
		    _numLR[1] = ((big - topEdge) >> 2) >> SampleStepLogY;
		}
		else
		{
			_startY[1] = topEdge + ((big - topEdge) >> 2);		
		    _numLR[1] = ((big - topEdge) >> 1) >> SampleStepLogY;
		}
		//找左边拟合点时的搜索起始纵坐标和搜索次数
		_startY[0] = small + ((downEdge - small) >> 2);		
	    _numLR[0] = ((downEdge - small) >> 1) >> SampleStepLogY;
	}
	return;
	
}
/*
 *  找左边
 *  @param [in] imageStart 图像开始地址
 *  @param [in] imageEnd 图像结束地址 
 *  @param [out] line 边线的斜率和切距的数组
 *  @return 无
 */
int FindLeftLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, searchNums, step , validN = 0, gradientTab, 
		smallGradient, bigGradient, pp, fixPoints = 0;
    Byte  *currentStart;
    int pGradient[200], pX[200], pY[200];

	searchNums = _numLR[0];//搜索次数
    currentStart = imageStart + _startY[0] * WidDpi;//开始采样的地址
    step = SampleStepY * WidDpi; //采样点间地址移动的位目
    gradientTab = (WidDpi - AbandanArrange)<<1;
	
	pp = _startY[0];
    //采样找边界点
    for (i = 0; i < searchNums; i++)
    {
		_pointPositionLR[1][i] = pp;//纵坐标
		_pointPositionLR[0][i] = RowFindPoint(currentStart , 1);//横坐标
		currentStart += step;//采样点下移
		pp += SampleStepY;

        if (i > 1)
        {
            //间隔点间葑甑牟?            
			if((_pointPositionLR[1][i - 2]!=-1)&&(_pointPositionLR[1][i]!=-1))
            {
                pGradient[i - 2] = _pointPositionLR[1][i - 2] - _pointPositionLR[1][i];
                pX[validN++] = pGradient[i - 2];
            }
            else
                pGradient[i - 2] = gradientTab;
        }
    }
    if(validN < 5)//成功采样过少
        return 1;
    SortArray(pX, validN);//排序
    smallGradient = pX[validN>> 2];//四分位数
    bigGradient = pX[validN - (validN>> 2)];
	
	step = searchNums - 2;
    if (smallGradient == 0 && bigGradient == 0) //如果元素横坐标无差异则认为直线垂直
    {
        //取一个点的横坐标
        for (i = 0; i < step; i++)
        {
            if (pGradient[i] == 0)
            {
                pp = _pointPositionLR[0][i];
                break;
            }
        }
        line[0] = -1000;
        line[1] = 1000.0f * pp;
    }
    else
    {
        //筛查出用于拟合的点
        for (i = 0; i < step; i++)
        {
            if ((pGradient[i] <= bigGradient)
                    && (pGradient[i] >= smallGradient))
            {
                pX[fixPoints] = _pointPositionLR[0][i];//用于拟合的横坐标
                pY[fixPoints++] = _pointPositionLR[1][i];//用于拟合的纵坐标
            }
        }

        fixLine(pY, pX, fixPoints, line);//拟毕?

        if (fabsf(line[0]) > 0.001)
        {
            line[1] /= (-line[0]);
            line[0] = 1 / line[0];
        }
        else
        {
            line[0] = -1000;
            line[1] = 1000.0f * pX[0];
        }
    }
    return 0;
}
/*
 *  找右边
 *  @param [in] imageStart 图像开始地址
 *  @param [in] imageEnd 图像结束地址 
 *  @param [out] line 边线的斜率和切距的数组
 *  @return 无
 */
int FindRightLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, searchNums, step, validN = 0, gradientTab,smallGradient, bigGradient, pp, fixPoints = 0;
    Byte *currentStart;
    int pGradient[200], pX[200], pY[200];

	searchNums = _numLR[1];//搜索次数
    currentStart = imageStart + (_startY[1] + 1) * WidDpi - AbandanArrange - 1;//开始采样的地址
    step = SampleStepY * WidDpi; //采样点间地址移动的位目
    gradientTab = (WidDpi - AbandanArrange)<<1;
	
	pp = _startY[1];
    //采样找边界点
    for (i = 0; i < searchNums; i++)
    {
        _pointPositionLR[1][i] = pp;//纵坐标
		_pointPositionLR[0][i] = RowFindPoint(currentStart , -1);//横坐标
		currentStart += step;//采样点下移
		pp += SampleStepY;
        if (i > 1)
        {
            //间隔点间纵坐标的差
            if((_pointPositionLR[1][i - 2]!=-1)&&(_pointPositionLR[1][i]!=-1))
            {
                pGradient[i - 2] = _pointPositionLR[1][i - 2] - _pointPositionLR[1][i];
                pX[validN++] = pGradient[i - 2];
            }
            else
                pGradient[i - 2] = gradientTab;
        }
    }
    if(validN < 5)//成功采样过少返回
        return 1;
    SortArray(pX, validN);//排序
    smallGradient = pX[validN>> 2];//四分位数
    bigGradient = pX[validN - (validN>> 2)];
    
	step = searchNums - 2;
    if (smallGradient == 0 && bigGradient == 0) //如果元素横坐标无差异则认为直线垂直
    {
        //取一个点的横坐标
        for (i = 0; i < step; i++)
        {
            if (pGradient[i] == 0)
            {
                pp = _pointPositionLR[0][i];
                break;
            }
        }
        line[0] = -1000;
        line[1] = 1000.0f * pp;
    }
    else
    {
        //筛查出用于拟合的点
        for (i = 0; i < step; i++)
        {
            if ((pGradient[i] <= bigGradient)
                    && (pGradient[i] >= smallGradient))
            {
                pX[fixPoints] = _pointPositionLR[0][i];//用于拟合的横坐标
                pY[fixPoints++] = _pointPositionLR[1][i];//用于拟合的纵坐标
            }
        }

        fixLine(pY, pX, fixPoints, line);//拟合直线

        if (fabsf(line[0]) > 0.001)
        {
            line[1] /= (-line[0]);
            line[0] = 1 / line[0];
        }
        else
        {
            line[0] = -1000;
            line[1] = 1000.0f * pX[0];
        }
    }
    return 0;
}

//对数组升序排列
void SortArray(int *a, int n)
{
	int k=n-1,j,kk,temp;  //k为最后一次调换的位置,kk为临时计数器
	while(k>0)
	{
		kk=0;
		for(j=0;j<k;j++)
		{
			if(a[j]>a[j+1])
			{
				temp=a[j];
				a[j]=a[j+1];
				a[j+1]=temp;
				kk=j;
			}
		}
		k=kk;
	}
}

//用最小二乘法拟合直线 ,斜率k 和b
void fixLine(int *x, int *y, int n, float *p)
{
    float A, B, k, b, sumx = 0.0, sumy = 0.0, sumxy = 0.0, sumxx = 0.0;
    int i;

    for (i = 0; i < n; i++)
    {
        sumxx = sumxx + x[i] * x[i];
        sumxy = sumxy + x[i] * y[i];
        sumx = sumx + x[i];
        sumy = sumy + y[i];
    }
    A = n * sumxy - sumx * sumy;
    B = n * sumxx - sumx * sumx;
    if (B == 0)
        k = A / (B + 0.0001f);
    else
        k = A / B;
    b = (sumy - k * sumx) / n;
    p[0] = k;
    p[1] = b;

}
/*
*求交点XY坐标
*lA[0]=k1,lA[1]=b1
*lB[0]=k2,lB[1]=b2
*p[0]=x0,p[1]=y0
*/
void CalcCrossPoint(float * lA, float *lB, int *p)
{
    float pp;
    pp = lB[0] - lA[0];
    if (pp == 0)
        pp = pp + 0.0001f;
    *p = (int)(lA[1] - lB[1]) / pp;
    *(p + 1) = (lA[1] * lB[0] - lA[0] * lB[1]) / pp;
}
//校准上下左右边
int CorrectLines(float up[], float down[], float left[], float right[], float k, int flag)
{
	float temp,temp1, temp2;

	//左右边误差过大 返回
	temp = (left[0] - right[0])/(1 + left[0]*right[0]);
	if(temp > k || temp < -k)
	{
		//解决结构过宽 美元图像右边异常问题 
		if(flag)
		{
			right[0] = left[0];
			if(up[0] > 0)
			{
				right[1] = _pointPositionLR[1][_numLR[1]-1] - right[0]* _pointPositionLR[0][_numLR[1]-1]; 
			}
			else
			{
				right[1] = _pointPositionLR[1][0] - right[0]* _pointPositionLR[0][0];
			}	
		}
		else 
			return 1; 
	}	
	//上下边误差过大时根据以与左右边最垂直的边作为参考进行校准	
	temp = (up[0] - down[0])/(1 + up[0]*down[0]);	
	if(temp > k || temp < -k)
	{
		temp = (left[0] + right[0])/2;
		temp1 = temp * up[0] + 1;
		temp1 = (temp1>0)?temp1:-temp1; //上边与左右边的垂直程度
		temp2 = temp*down[0] + 1;
		temp2 = (temp2>0)?temp2:-temp2; //下边与左右边的垂直程度
		if(temp1 < temp2)//上边与左右边更垂直
		{
			down[0] = up[0];
			down[1] = DownValidPoint[1] - down[0]*DownValidPoint[0];
		}
		else
		{
			up[0] = down[0];
			up[1] = UpValidPoint[1] - up[0]*UpValidPoint[0];
		}
		
	}
	return 0;
}

int FindPoints(Byte * imageStart, Byte * imageEnd, int pointLocation[],int flag)
{
    int smallY, bigY, orient;
    float upLine[2], downLine[2], leftLine[2], rightLine[2];

    _height = (imageEnd - imageStart) / WidDpi - 1;//图像高度
    if(FindUpLine(imageStart, imageEnd, upLine))//上边
        return 1;
    if(FindDownLine(imageStart, imageEnd, downLine))//下边
        return 1;
    if (upLine[0] > 0)
    {
        smallY = upLine[0] * (WidDpi - AbandanArrange - 1) + upLine[1];//上边与图像左右边相交的最低点
        bigY = downLine[1];			//下边与图像左右边相交的最高点 
		orient = 1;					//钞票上边倾斜方向（斜率大于0为1小于0为0）
    }
    else
    {
        smallY = upLine[1];
        bigY = downLine[0] * (WidDpi - AbandanArrange - 1) + downLine[1];
		orient = 0;
    }
	if(smallY >= _height||bigY <= 0)
		return 1;
	//确定搜索左右边的位置和搜索次数
	DefineSearchLR(imageStart, orient, smallY, bigY,flag);
	
	//钞票倾斜不超过40°
	if(upLine[0] > 0.8391||upLine[0] < -0.8391)
	{
		return 1;
	}
    if((_numLR[0]<=0)||(_numLR[0]>200)||(_numLR[1]<=0)||(_numLR[1]>200))
        return 1;
    if(FindLeftLine(imageStart, imageEnd, leftLine))//左边
        return 1;
    if(FindRightLine(imageStart, imageEnd, rightLine))//右边
        return 1;

	//校准四条边方向 使平行的线误差不大于8°
	if(CorrectLines(upLine, downLine, leftLine, rightLine, 0.2679,flag))
		return 1;
	cout << _height << endl;
	cout << "orient"<<orient<<endl;
	cout << "upline:y=" << upLine[0] << "x+" << upLine[1] << endl;
	cout << "downLine:y=" << downLine[0] << "x+" << downLine[1] << endl;
	cout << "leftLine:y=" << leftLine[0] << "x+" << leftLine[1] << endl;
	cout << "rightLine:y=" << rightLine[0] << "x+" << rightLine[1] << endl << endl;
    //计算交点
    CalcCrossPoint(upLine, leftLine, pointLocation);//上边和交
    CalcCrossPoint(upLine, rightLine, pointLocation + 2);//上边和右边交点
    CalcCrossPoint(downLine, rightLine, pointLocation + 4);//下边和右边交点
    CalcCrossPoint(downLine, leftLine, pointLocation + 6);//下边和左边交点
    return 0;

}