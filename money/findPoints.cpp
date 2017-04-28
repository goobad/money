#include "findPoints.h"
#include <iostream>
using namespace std;
int _height;//ͼ��߶�
int _startY[2];//���ұ߲�����ʼ��y����,_startY[0]Ϊ��ߵ�
int _numLR[2];//���ұ߲���������_numLR[0]Ϊ��ߵ�
int _pointPosition[2][SamplePoints];//���±߽��
int _pointPositionLR[2][200];//���ұ߽��
int UpValidPoint[2],DownValidPoint[2];//���±���Ч��
int pointLocation[8];
int pointLocation1[8];

extern struct boardConfig configSet;

void initPointPosition ()
{
	int i;
    for (i = 0; i < SamplePoints; i++)
    {
        _pointPosition[0][i] = StartX + (i << SampleStepLogX);//������
    }
}

/*
 *  ÿһ������߽��
 *  @param [in] imageStart �п�ʼ����
 *  @param [in] position ����1���ϱ� -1���±�  
 *  @return �߽�Y����
 */
 int ColumnFindPoint(Byte * start, int position)
 {
	int i,j,pointY;
	int bigSearchNum = _height/BigSearchY - 1, smallSearchNum = 2*BigSearchY/SmallSearchY, searchNum = 2*SmallSearchY;//��С�����������Ĵ���
	int bigBackPoints = position*WidDpi*(2*BigSearchY - SmallSearchY);//��������������
	int smallBackPoints = position*WidDpi*(2*SmallSearchY - 1);//С������������
	int bigStep = position*BigSearchY*WidDpi,smallStep = position*SmallSearchY*WidDpi,step = position*WidDpi;//��С�����в���
	Byte th = configSet.ValueThreshold;
	Byte *pr = start;
	//Byte th = 50;
	//ͷBigSearchY�����⴦��
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
	
	//ʣ����	
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
 *  ÿһ������߽��
 *  @param [in] imageStart �п�ʼ����
 *  @param [in] position ����1����� -1���ұ�  
 *  @return �߽�X����
 */
 int RowFindPoint(Byte * start, int position)
 {
	int i,j,pointX;
	int bigSearchNum = (WidDpi - AbandanArrange)/BigSearchX - 3, smallSearchNum = 2*BigSearchX/SmallSearchX, searchNum = 2*SmallSearchX;//��С�����������Ĵ���
	int bigBackPoints = position*(2*BigSearchX - SmallSearchX);//��������������
	int smallBackPoints = position*(2*SmallSearchX - 1);//С������������
	int bigStep = position*BigSearchX,smallStep = position*SmallSearchX,step = position;//��С�����в���
	int dotStep = position*DotSize;//���ǻ��ҵĴ�С
	Byte th = configSet.ValueThreshold, *pr = start;
	
	//ͷBigSearchX�����⴦��
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
	
	//ʣ����	
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
 *  ���ϱ�
 *  @param [in] imageStart ͼ��ʼ��ַ
 *  @param [in] imageEnd ͼ�������ַ 
 *  @param [out] line ���ߵ�б�ʺ��о������
 *  @return ��
 */
int FindUpLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, n, validN = 0, gradientTab,smallGradient, bigGradient,  fixPoints = 0;
    Byte *currentStart;
    int pGradient[SamplePoints], pX[SamplePoints], pY[SamplePoints];

    currentStart = imageStart + StartX;//��ʼ�����ĵ�ַ
    gradientTab = _height<<1;
    //�����ұ߽��
    for (i = 0; i < SamplePoints; i++)
    {
	
		//ÿ�ж����ж�������Ծ�ұ�
		_pointPosition[1][i] = ColumnFindPoint(currentStart + (i << SampleStepLogX), 1);
		
		//�����������ݶ������޳��쳣��
        if (i > 1)
        {
            //������������Ĳ�
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
        return 1;//�����ɹ��ĵ�������
    SortArray(pX, validN);//����
    smallGradient = pX[validN>> 2];//�ķ�λ��
    bigGradient = pX[validN - (validN>> 2)];
    //ɸ���������ϵĵ�
	n = SamplePoints - 2;
    for (i = 0; i < n; i++)
    {
        if ((pGradient[i] <= bigGradient) && (pGradient[i] >= smallGradient))
        {
            pX[fixPoints] = _pointPosition[0][i];//������ϵĺ�����
            pY[fixPoints++] = _pointPosition[1][i];//������ϵ�������
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

    fixLine(pX, pY, fixPoints, line);//���ֱ��
	UpValidPoint[0] = pX[0];//��¼��һ����Ч��
	UpValidPoint[1] = pY[0];
    return 0;
}
/*
 *  ���±�
 *  @param [in] imageStart ͼ��ʼ��ַ
 *  @param [in] imageEnd ͼ�������ַ 
 *  @param [out] line ���ߵ�б�ʺ��о������
 *  @return ��
 */
int FindDownLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, n, validN = 0, gradientTab, smallGradient, bigGradient, fixPoints = 0;
    Byte *currentStart;
    int pGradient[SamplePoints], pX[SamplePoints], pY[SamplePoints];

    currentStart = imageEnd - WidDpi + StartX;//��ʼ�����ĵ�ַ
    gradientTab = _height<<1;
    //�����ұ߽��
    for (i = 0; i < SamplePoints; i++)
    {

		//ÿ�ж����ж�������Ծ�ұ�
		_pointPosition[1][i] = ColumnFindPoint(currentStart + (i << SampleStepLogX), -1);
		
        if (i > 1)
        {
            //������������Ĳ�
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
    SortArray(pX, validN);//����
    smallGradient = pX[validN>> 2];//�ķ�λ��
    bigGradient = pX[validN - (validN>> 2)];
    //ɸ���������ϵĵ�
	n = SamplePoints - 2;
    for (i = 0; i < n; i++)
    {
        if ((pGradient[i] <= bigGradient) && (pGradient[i] >= smallGradient))
        {
            pX[fixPoints] = _pointPosition[0][i];//������ϵĺ�����
            pY[fixPoints++] = _pointPosition[1][i];//������ϵ�������
        }
    }
    fixLine(pX, pY, fixPoints, line);//���ֱ��
	DownValidPoint[0] = pX[0];//��¼��һ����Ч��
	DownValidPoint[1] = pY[0];
    return 0;
}
/*
 *  �ҳ�Ʊ������ʼ�߽�
 *  @param [in] imageStart ͼ��ʼ��ַ
 *  @param [in] orient ��Ʊ�ϱ���б����б�ʴ���0Ϊ1С��0Ϊ0��
 *  @param [in] small �ϱ���ͼ�����ұ��ཻ����͵�
 *  @param [in] big �±���ͼ�����ұ��ཻ����ߵ�
 *  @return ��
 */
void DefineSearchLR(Byte * imageStart, char orient,int small, int big,int flag)
{
	int i,j,horizonStep, doubleStep,verticStep, topSearchN,downSearchN,horizonSearchN;
	int topEdge, downEdge;
	Byte * currentT, *currentD, *pp;
	topSearchN = small/5;		  //��������������Ͻ�����ѭ������
	downSearchN = (_height-big)/5;//��������������½�����ѭ������
	horizonSearchN = (WidDpi - AbandanArrange)/10 - 2;	  //���10�������ұ߽�����ѭ������
	verticStep = 5*WidDpi;		  //���򲽳�
	//��ʼ�����½�
	topEdge = 0;		//�Ͻ�
	downEdge= _height;	//��
	
	if(orient)
	{
		currentT = imageStart + small*WidDpi;		//��Ͻ���������?		
		currentD = imageStart + (big+1)*WidDpi - 1 - AbandanArrange;	//���½��������ַ
		horizonStep = 10;//ˮƽ����
		doubleStep = horizonStep<<1;//����ˮƽ����
	}
	else
	{
		currentT = imageStart + (small+1)*WidDpi - 1 - AbandanArrange;
		currentD = imageStart + big*WidDpi;
		horizonStep = -10;
		doubleStep = horizonStep<<1;
	}
	//������ʼ�߽�
	for(i = 0; i<topSearchN;i++ )
	{
		pp = currentT;
		for(j=0;j<horizonSearchN;j++ )
		{
			//�����ǰ�㼰ƽ�Ʋ�����������һ���㶼���Ǳ������м�������
			if((*pp > configSet.ValueThreshold)&&((*(pp+horizonStep) >
				configSet.ValueThreshold)||(*(pp+doubleStep) > configSet.ValueThreshold)))
				break;
			pp += horizonStep;
		}
		//�����һ���Ǳ������ҵ�����ʼ�߽磬��¼��Y����
		if(j == horizonSearchN)
		{
			topEdge = small - (i+1)*5;//��¼���½�
			break;
		}
		currentT -=verticStep;
	}
	//������ʼ�߽�
	for(i = 0; i<downSearchN;i++ )
	{
		pp = currentD;
		for(j=0;j<horizonSearchN;j++ )
		{
			//�����ǰ�㼰ƽ�Ʋ�����������һ���㶼���Ǳ������м�������
			if((*pp > configSet.ValueThreshold)&&((*(pp-horizonStep) > 
				configSet.ValueThreshold)||(*(pp-doubleStep) > configSet.ValueThreshold)))
				break;
			pp -= horizonStep;
		}
		//�����һ���Ǳ���
		if(j == horizonSearchN)
		{
			downEdge = big + (i-1)*5;//��¼���½�
			break;
		}
		currentD +=verticStep;
	}
	if(orient)//��Ʊ�ϱ�б�ʴ���0
	{
		//�������ϵ�ʱ��������ʼ���������������
		_startY[0] = topEdge + ((big - topEdge) >> 2);		
	    _numLR[0] = ((big - topEdge) >> 1) >> SampleStepLogY;
		
		//���ұ���ϵ�ʱ��������ʼ�������������?
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
		//���ұ���ϵ�ʱ��������ʼ���������������
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
		//�������ϵ�ʱ��������ʼ���������������
		_startY[0] = small + ((downEdge - small) >> 2);		
	    _numLR[0] = ((downEdge - small) >> 1) >> SampleStepLogY;
	}
	return;
	
}
/*
 *  �����
 *  @param [in] imageStart ͼ��ʼ��ַ
 *  @param [in] imageEnd ͼ�������ַ 
 *  @param [out] line ���ߵ�б�ʺ��о������
 *  @return ��
 */
int FindLeftLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, searchNums, step , validN = 0, gradientTab, 
		smallGradient, bigGradient, pp, fixPoints = 0;
    Byte  *currentStart;
    int pGradient[200], pX[200], pY[200];

	searchNums = _numLR[0];//��������
    currentStart = imageStart + _startY[0] * WidDpi;//��ʼ�����ĵ�ַ
    step = SampleStepY * WidDpi; //��������ַ�ƶ���λĿ
    gradientTab = (WidDpi - AbandanArrange)<<1;
	
	pp = _startY[0];
    //�����ұ߽��
    for (i = 0; i < searchNums; i++)
    {
		_pointPositionLR[1][i] = pp;//������
		_pointPositionLR[0][i] = RowFindPoint(currentStart , 1);//������
		currentStart += step;//����������
		pp += SampleStepY;

        if (i > 1)
        {
            //�����������Ĳ?            
			if((_pointPositionLR[1][i - 2]!=-1)&&(_pointPositionLR[1][i]!=-1))
            {
                pGradient[i - 2] = _pointPositionLR[1][i - 2] - _pointPositionLR[1][i];
                pX[validN++] = pGradient[i - 2];
            }
            else
                pGradient[i - 2] = gradientTab;
        }
    }
    if(validN < 5)//�ɹ���������
        return 1;
    SortArray(pX, validN);//����
    smallGradient = pX[validN>> 2];//�ķ�λ��
    bigGradient = pX[validN - (validN>> 2)];
	
	step = searchNums - 2;
    if (smallGradient == 0 && bigGradient == 0) //���Ԫ�غ������޲�������Ϊֱ�ߴ�ֱ
    {
        //ȡһ����ĺ�����
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
        //ɸ���������ϵĵ�
        for (i = 0; i < step; i++)
        {
            if ((pGradient[i] <= bigGradient)
                    && (pGradient[i] >= smallGradient))
            {
                pX[fixPoints] = _pointPositionLR[0][i];//������ϵĺ�����
                pY[fixPoints++] = _pointPositionLR[1][i];//������ϵ�������
            }
        }

        fixLine(pY, pX, fixPoints, line);//���?

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
 *  ���ұ�
 *  @param [in] imageStart ͼ��ʼ��ַ
 *  @param [in] imageEnd ͼ�������ַ 
 *  @param [out] line ���ߵ�б�ʺ��о������
 *  @return ��
 */
int FindRightLine(Byte * imageStart, Byte * imageEnd, float *line)
{
    int i, searchNums, step, validN = 0, gradientTab,smallGradient, bigGradient, pp, fixPoints = 0;
    Byte *currentStart;
    int pGradient[200], pX[200], pY[200];

	searchNums = _numLR[1];//��������
    currentStart = imageStart + (_startY[1] + 1) * WidDpi - AbandanArrange - 1;//��ʼ�����ĵ�ַ
    step = SampleStepY * WidDpi; //��������ַ�ƶ���λĿ
    gradientTab = (WidDpi - AbandanArrange)<<1;
	
	pp = _startY[1];
    //�����ұ߽��
    for (i = 0; i < searchNums; i++)
    {
        _pointPositionLR[1][i] = pp;//������
		_pointPositionLR[0][i] = RowFindPoint(currentStart , -1);//������
		currentStart += step;//����������
		pp += SampleStepY;
        if (i > 1)
        {
            //������������Ĳ�
            if((_pointPositionLR[1][i - 2]!=-1)&&(_pointPositionLR[1][i]!=-1))
            {
                pGradient[i - 2] = _pointPositionLR[1][i - 2] - _pointPositionLR[1][i];
                pX[validN++] = pGradient[i - 2];
            }
            else
                pGradient[i - 2] = gradientTab;
        }
    }
    if(validN < 5)//�ɹ��������ٷ���
        return 1;
    SortArray(pX, validN);//����
    smallGradient = pX[validN>> 2];//�ķ�λ��
    bigGradient = pX[validN - (validN>> 2)];
    
	step = searchNums - 2;
    if (smallGradient == 0 && bigGradient == 0) //���Ԫ�غ������޲�������Ϊֱ�ߴ�ֱ
    {
        //ȡһ����ĺ�����
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
        //ɸ���������ϵĵ�
        for (i = 0; i < step; i++)
        {
            if ((pGradient[i] <= bigGradient)
                    && (pGradient[i] >= smallGradient))
            {
                pX[fixPoints] = _pointPositionLR[0][i];//������ϵĺ�����
                pY[fixPoints++] = _pointPositionLR[1][i];//������ϵ�������
            }
        }

        fixLine(pY, pX, fixPoints, line);//���ֱ��

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

//��������������
void SortArray(int *a, int n)
{
	int k=n-1,j,kk,temp;  //kΪ���һ�ε�����λ��,kkΪ��ʱ������
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

//����С���˷����ֱ�� ,б��k ��b
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
*�󽻵�XY����
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
//У׼�������ұ�
int CorrectLines(float up[], float down[], float left[], float right[], float k, int flag)
{
	float temp,temp1, temp2;

	//���ұ������� ����
	temp = (left[0] - right[0])/(1 + left[0]*right[0]);
	if(temp > k || temp < -k)
	{
		//����ṹ���� ��Ԫͼ���ұ��쳣���� 
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
	//���±�������ʱ�����������ұ��ֱ�ı���Ϊ�ο�����У׼	
	temp = (up[0] - down[0])/(1 + up[0]*down[0]);	
	if(temp > k || temp < -k)
	{
		temp = (left[0] + right[0])/2;
		temp1 = temp * up[0] + 1;
		temp1 = (temp1>0)?temp1:-temp1; //�ϱ������ұߵĴ�ֱ�̶�
		temp2 = temp*down[0] + 1;
		temp2 = (temp2>0)?temp2:-temp2; //�±������ұߵĴ�ֱ�̶�
		if(temp1 < temp2)//�ϱ������ұ߸���ֱ
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

    _height = (imageEnd - imageStart) / WidDpi - 1;//ͼ��߶�
    if(FindUpLine(imageStart, imageEnd, upLine))//�ϱ�
        return 1;
    if(FindDownLine(imageStart, imageEnd, downLine))//�±�
        return 1;
    if (upLine[0] > 0)
    {
        smallY = upLine[0] * (WidDpi - AbandanArrange - 1) + upLine[1];//�ϱ���ͼ�����ұ��ཻ����͵�
        bigY = downLine[1];			//�±���ͼ�����ұ��ཻ����ߵ� 
		orient = 1;					//��Ʊ�ϱ���б����б�ʴ���0Ϊ1С��0Ϊ0��
    }
    else
    {
        smallY = upLine[1];
        bigY = downLine[0] * (WidDpi - AbandanArrange - 1) + downLine[1];
		orient = 0;
    }
	if(smallY >= _height||bigY <= 0)
		return 1;
	//ȷ���������ұߵ�λ�ú���������
	DefineSearchLR(imageStart, orient, smallY, bigY,flag);
	
	//��Ʊ��б������40��
	if(upLine[0] > 0.8391||upLine[0] < -0.8391)
	{
		return 1;
	}
    if((_numLR[0]<=0)||(_numLR[0]>200)||(_numLR[1]<=0)||(_numLR[1]>200))
        return 1;
    if(FindLeftLine(imageStart, imageEnd, leftLine))//���
        return 1;
    if(FindRightLine(imageStart, imageEnd, rightLine))//�ұ�
        return 1;

	//У׼�����߷��� ʹƽ�е���������8��
	if(CorrectLines(upLine, downLine, leftLine, rightLine, 0.2679,flag))
		return 1;
	cout << _height << endl;
	cout << "orient"<<orient<<endl;
	cout << "upline:y=" << upLine[0] << "x+" << upLine[1] << endl;
	cout << "downLine:y=" << downLine[0] << "x+" << downLine[1] << endl;
	cout << "leftLine:y=" << leftLine[0] << "x+" << leftLine[1] << endl;
	cout << "rightLine:y=" << rightLine[0] << "x+" << rightLine[1] << endl << endl;
    //���㽻��
    CalcCrossPoint(upLine, leftLine, pointLocation);//�ϱߺͽ�
    CalcCrossPoint(upLine, rightLine, pointLocation + 2);//�ϱߺ��ұ߽���
    CalcCrossPoint(downLine, rightLine, pointLocation + 4);//�±ߺ��ұ߽���
    CalcCrossPoint(downLine, leftLine, pointLocation + 6);//�±ߺ���߽���
    return 0;

}