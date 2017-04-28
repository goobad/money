#include "characterDetector.h"

extern struct boardConfig configSet;
extern int BWMiniImage[];
extern int GrayScaleTh;
extern int GrayRange;
extern GZHType pRecord;			//冠字号存储

Byte singleChar[450],singleChar_Gray[450];

inline void addPosition (int * position, int index, int start, int end)
{
	position[(index)<<1] = start;
	position[((index)<<1) + 1] = end;
}

char findBoarder(unsigned char * BWMiniImage, int * positionX, 
				 int * positionY, int defaultCharNum)
{
    int i, j, k, num, spaceWidth,
		startX, endX, startY, endY, startP, endP,
		width, height, tempPositionX[30];
    int validStartY, validEndY, *pInt = (int *)BWMiniImage;
	unsigned int sum[GZHWidth];
    Byte *pByte;
	int dirtyindex = -1;
    
	//横向投影
	//memset (sum,0,GZHHeight*4);
	
	memset (positionX,0xFF,26*4);
	memset (positionY,0xFF,26*4);
	
	pInt = (int *)BWMiniImage;
	for (i = 0; i < GZHHeight; i+=2)
	{
		sum[i] = 0;
		for (j = 0; j < GZHqWidth; j++)
			sum[i] += *(pInt++);
		pInt += GZHqWidth;
		sum[i] = ((sum[i]>>0)&0xFF) + ((sum[i]>>8)&0xFF) + ((sum[i]>>16)&0xFF) 
				+ ((sum[i]>>24)&0xFF);
		sum[i] = (sum[i]>8);
	}
	
	i = 0;
	height = 0;
	
	while (height < MinCharHeight && i < GZHHeight)
	{
		//找开头
		while (i < GZHHeight && !(sum[i] && sum[i+2]))
			i+=2;
		validStartY = (i > 2)?(i-2):0;
		//找结尾
		while (i < GZHHeight && (sum[i] || sum[i+2]))
			i+=2;
		validEndY = (i+2 < GZHHeight)?(i+2):GZHHeight;
		height = validEndY - validStartY;
	}
   
    //纵向投影
    memset(sum, 0, GZHWidth);
	pInt = (int *)(BWMiniImage + validStartY * GZHWidth); //指针移到字符水平起始位置	
    for (i = validStartY; i < validEndY; i++)
        for (j = 0; j < GZHqWidth; j++)
            sum[j] += *(pInt++);//纵向投影
	
	num = 0;	
	pByte=(Byte *)sum;
	//memset (dirty,0,15);
	//横向分割
	for (i = 1; (i < GZHWidth) && (num < 15);)
	{
		while (i < GZHWidth&&!(pByte[i]&&pByte[i+1]))
			i++;
		startX = i;
		while (i < GZHWidth&&(pByte[i]||pByte[i+1]))
			i++;
		endX = i;
		
		width = endX - startX;
		if (width < MinCharWidth)
		{
			k = 0;
			for (j = startX; j < endX; j++)
				k += pByte[j];
			if (k >= 18)
				addPosition(tempPositionX,num++,startX,endX);
		}
		else
		{
			if (width < MaxCharWidth)
			{
				addPosition(tempPositionX,num++,startX,endX);
			}
			else
			{
				if (width < (MaxTwoCharWidth))
				{	
					//两个字符连起来的时候，假设他们宽度相等
					//暂时不考虑三个字符的情况
					width = (width - MinSpaceWidth)>>1;
					addPosition(tempPositionX,num++,startX,startX+width);
					addPosition(tempPositionX,num++,endX - width,endX);
				}
				else
				{
					//dirty[num] = 1;
					
					addPosition(tempPositionX,num++,startX,endX);
					dirtyindex = num;
				}
			}
		}
	}

	//过滤字符宽度异常的记录
	//前1/4
	for (i = num>>2; i > 0; i--)
	{
		spaceWidth = tempPositionX[i << 1] - tempPositionX[(i << 1) - 1];
		width = ((tempPositionX[(i << 1) + 1] - tempPositionX[(i << 1)] ) < 7)
			  ||((tempPositionX[(i << 1) - 1] - tempPositionX[(i << 1) - 2] ) < 7);
		if ((spaceWidth > MaxSpaceWidth) && !width)
			break;
	}
	startP = i;
	//末尾1/4
	for (i = (num>>1) + (num>>2); i < num; i++)
	{
		spaceWidth = tempPositionX[i << 1] - tempPositionX[(i << 1) - 1];
		
		width = ((tempPositionX[(i << 1) + 1] - tempPositionX[(i << 1)] ) < 7)
			  ||((tempPositionX[(i << 1) - 1] - tempPositionX[(i << 1) - 2] ) < 7);
		
		if (spaceWidth > MaxSpaceWidth && !width)
			break;
	}
	endP = i;
	
	//过滤剩下的字符数量


	//过滤剩下的字符数量
	if ((endP - startP) < MinCharNums || (endP - startP) > defaultCharNum+2)
    {
        return FALSE;
    }
	//字符不够，填*
	if ( dirtyindex != -1 && (endP - startP) < defaultCharNum && dirtyindex >= startP 
		&& dirtyindex < endP)
	{
		startX = dirtyindex*2;
		endX =  (startP + defaultCharNum - (endP - dirtyindex))*2;
		
		width = (endP - dirtyindex)*8;
		
		memcpy (positionX,				tempPositionX + startX,	width);
		memcpy (tempPositionX + endX,	positionX,				width);
		
		startX -= 2;
		
		for (i = startX; i < endX; i ++ )
			tempPositionX[i] = -1;
		
		endP += (endX - startX - 2)/2;
	}

	
	num = 0;
	//寻找每一块的上下边缘
	for (i = startP; i < endP; i++)
	{
		startX = tempPositionX[i << 1];
		endX = tempPositionX[(i << 1) + 1];
		
		//*
		if (startX < 0)
		{
			addPosition(positionX,num,-1,-1);
			addPosition(positionY,num++,-1,-1);
			continue;
		}
			
		
		pByte = BWMiniImage + validStartY * GZHWidth;
		//横向投影
		for (j = validStartY; j < validEndY; j++)
		{
			sum[j] = 0;
			for (k = startX; k < endX; k ++)
				sum[j] += pByte[k];
			pByte += GZHWidth;
		}
		
		//扫描字符上下界
		j = validStartY;
		height = 0;
		while ((j < validEndY) && (height < MinCharHeight))
		{
			while ((j < validEndY) && !((sum[j] > 1)&&(sum[j+1] > 0)))
				j++;
			startY = j;

			j = validEndY;
			while ((j > startY) && !((sum[j] > 0)&&(sum[j-1] > 0)))
				j--;

			endY = j + 1;
			
			height = endY - startY;
		}
		
		//添加字符
		if (height >= MinCharHeight && height <= MaxCharHeight)
		{
			addPosition(positionX,num,startX,endX);
			addPosition(positionY,num++,startY,endY);
		}
		else if (endX - startX > 10 && height > MinCharHeight)
		{
			addPosition(positionX,num,-1,-1);
			addPosition(positionY,num++,startY,endY);
		}
	}

	return endP - startP;
}

int idenGZHRMB (Byte *miniImage, char result[], int direction, 
				int * edition,  Byte * GZHImage,int money)
{
    int wwww, positionX[26], positionY[26];	
    Byte  thresh[4];
	char *result_all = result;
	Byte gray[2];
	int charNums = 0; 

	switch (direction)
	{
		case 1:
			gray[0] = configSet.valueGBblack[3];
			gray[1] = configSet.valueGBwhite[3];
			break;
		case 2:
			gray[0] = configSet.valueGBblack[1];
			gray[1] = configSet.valueGBwhite[1];
			break;
		case 3:
			gray[0] = configSet.valueGBblack[6];
			gray[1] = configSet.valueGBwhite[6];
			break;
		case 4:
			gray[0] = configSet.valueGBblack[4];
			gray[1] = configSet.valueGBwhite[4];
			break;
	}
	GrayScaleTh = gray[0] + (gray[1] - gray[0]) * 0.70;
	GrayRange = (gray[1] - gray[0])*0.5;	
	
    NewGray2bw(miniImage, (Byte *)BWMiniImage, GZHHeight, GZHWidth,thresh);

	//NewGray2bw21(miniImage, (Byte *)BWMiniImage, GZHHeight, GZHWidth,thresh,255);

	//Gray2bw(miniImage, (Byte *)BWMiniImage, GZHHeight, GZHWidth,230);
    //filterSmallAreas ((Byte *)BWMiniImage);
	
    if (!(charNums = findBoarder((Byte *)BWMiniImage, positionX, positionY, 10)))
       	return 0;//???????

	if ((charNums > 12) || (positionX[19] > GZHWidth))
	    return 1;//????????????
		
	if (charNums > 10)
		charNums = 10;

	
	if (*edition == EDITION_UNKNOW || *edition==EDITION_RMB2005|| *edition==EDITION_RMB1999) 
	{
		wwww = (positionY[1] - positionY[0]) + (positionY[3] - positionY[2])
			 - (positionY[9] - positionY[8]) - (positionY[11] - positionY[10]);
		*edition = (wwww>1)?EDITION_RMB1999:EDITION_RMB2005;
	}

//	if()
	switch (*edition)
	{
		case EDITION_RMB1999:
			if((money == 100)||(money == 1))
			{
				thresh[0] = 0.9*thresh[0];
				thresh[1] = 0.9*thresh[1];
				thresh[2] = 0.9*thresh[2];
				thresh[3] = 0.9*thresh[3];
			}
			return idenRMB99 (miniImage, positionX, positionY, thresh, 
				result_all, GZHImage,charNums);
		case EDITION_RMB2005:
			return idenRMB05 (miniImage, positionX, positionY, thresh, 
				result_all, GZHImage,charNums);
		case EDITION_RMB2015:
			return idenRMB15 (miniImage, positionX, positionY, thresh, 
				result_all, GZHImage,charNums);
		case EDITION_RMB1YUAN:
			return idenRMB1RMB (miniImage, positionX, positionY, thresh, 
				result_all, GZHImage,charNums);
	}
	
	return 0;
}

//分割出字符并按25:18比例缩放
int segmentChar(unsigned char * mimiImage, unsigned char destImage[450], 
				unsigned char destGrayImage[450],
				int * positionX, int * positionY, unsigned char *th)
{
    int i, j, m , mm, nn, deltaX, deltaY, pointX, pointY, u, v, sum = 0;
    unsigned char threshValue,*temp, value, threshValuel, lowImage[450];
    unsigned char * p = destImage,* q = lowImage, validPoints;

	//根据字符的位置决定使用的块阈值 当处中间时用两块的平均值
	u = positionX[1]/GZHqWidth;
	v = positionX[0]/GZHqWidth;
	if(u == v)
		threshValue = th[u];
	else
	{
		mm = th[u];
	    nn = th[v]; 
		threshValue = (mm + nn)/2;
	}		
	threshValuel = threshValue * 0.78125;
    //对每一块进行缩放
    deltaX = ((positionX[1] - positionX[0])<<10) / 18;
    deltaY = ((positionY[1] - positionY[0])<<10) / 25;
	mm = (positionX[0]<<10);
    pointY = (positionY[0]<<10);
    for (i = 0; i < 25; i++)
    {
        pointX = mm;
		nn = (pointY>>10)* GZHWidth;
		v = pointY&0x3ff;
        for (j = 0; j < 18; j++)
        {
            m = pointX>>10;//最近行
            temp = mimiImage + nn + m;
			u = pointX&0x3ff;
			
            //双线性插值
            value = ((((1024 -u) * *temp + u* *(temp + 1)) * (1024 -v) + ((1024 -u)
                         * *(temp + GZHWidth) + u * *(temp + GZHWidth+1))* v)>>20);
			*(destGrayImage++) = value;         //灰度图
			*(p++) = value<threshValue; //二值图
			*(q++) = value < threshValuel;      //低阈值二值图
			sum += value;
            pointX += deltaX;
        }
        pointY += deltaY;
    }

	p -= 412;
	q -= 412; 
	for(i=0;i<21;i++)
	{
		for(j=0;j<14;j++,p++,q++)
		{
			if (*p)
			{
				validPoints = (*(q - 38) +*(q - 37) +*(q - 36) +*(q - 35) +*(q - 34)
							  +*(q - 20)								  +*(q - 16)
							  +*(q - 2)	                                  +*(q + 2)
							  +*(q + 16) 							      +*(q + 20)
						      +*(q + 34) +*(q + 35) +*(q + 36) +*(q + 37) +*(q + 38));
				if (!validPoints)
					*p = 0;
			}
		}
		p +=4;
		q +=4;
	}
	return sum/(25*18);
}

//提取特征
void feature(unsigned char * segmentImage, short result[29])
{
    short state,state2,state3, i, j;
    unsigned char * temp,* temp2 ,* temp3,*temp4,*temp5;
    memset(result, 0, sizeof(short) * 26);
    //横向穿越 首尾不为0时也算穿越(3行为单位)
    for (i = 0; i < 8; i++)
    {    
		//三行
		temp = segmentImage + i*54;
		temp2 = temp+18;
		temp3 = temp2+18;
    
        state = state2 =state3 = 0;
        for (j = 0; j < 18; j++)
        {
            if (*temp == state)
                ;
            else
            {
                state = *temp;
                result[i] += 18;
            }
			if (*temp2 == state2)
                ;
            else
            {
                state2 = *temp2;
                result[i] += 18;
            }
			if (*temp3 == state3)
                ;
            else
            {
                state3 = *temp3;
                result[i] += 18;
            }
            temp++;temp2++;temp3++;
        }
        if (*(temp-1) != 0)
            result[i] += 18;
		if (*(temp2-1) != 0)
            result[i] += 18;
		if (*(temp3-1) != 0)
            result[i] += 18;			
    }
    //纵向穿越特性(3列为单位)
    for (i = 0; i < 6; i++)
    {
        //三列
        temp = segmentImage + 3 * i;
		temp2 = temp +1;
		temp3 = temp2+1;
		
        state = state2 =state3 = 0;
        for (j = 0; j < 25; j++)
        {
            if (*temp == state)
                ;
            else
            {
                state = *temp;
                result[8 + i] += 18;
            }
			if (*temp2 == state2)
                ;
            else
            {
                state2 = *temp2;
                result[8 + i] += 18;
            }
			if (*temp3 == state3)
                ;
            else
            {
                state3 = *temp3;
                result[8 + i] += 18;
            }
            temp += 18; temp2 += 18; temp3 += 18;
        }
		
        if (*(temp-18) != 0)
            result[8 + i] += 18;
		if (*(temp2-18) != 0)
            result[8 + i] += 18;
		if (*(temp3-18) != 0)
            result[8 + i] += 18;	
    }
    //分块和
    for (i = 0; i < 15; i++)
    {
        j = 90 * (i / 3) + 6 * (i % 3);
		temp = segmentImage+j;
		temp2 = temp+18;
		temp3 = temp2+18;
		temp4 = temp3+18;
		temp5 = temp4+18;
		
		result[14 + i] = *temp+ *(temp+1)+*(temp+2)+*(temp+3)+*(temp+4)+*(temp+5)
						+*temp2+ *(temp2+1)+*(temp2+2)+*(temp2+3)+*(temp2+4)+*(temp2+5)
						+*temp3+ *(temp3+1)+*(temp3+2)+*(temp3+3)+*(temp3+4)+*(temp3+5)
						+*temp4+ *(temp4+1)+*(temp4+2)+*(temp4+3)+*(temp4+4)+*(temp4+5)
						+*temp5+ *(temp5+1)+*(temp5+2)+*(temp5+3)+*(temp5+4)+*(temp5+5);
						
        
		result[14 + i] *= 29;
    }
}

char ocrcore (unsigned char * in, char Type, int edition)
{
	short features[29];
	const short (* Digitfeature)[29],(* Charfeature)[29];
	Digitfeature = NULL;//by lrj
	Charfeature = NULL;//by lrj
	char temp;
	/*
	if (Template != fun1(Template2))
	{
		if (rand () &0xff == 0)
		{
			temp = 'A'+rand()%36;
			if (temp > 'Z')
				temp = '0' + (temp - 'Z');
			return temp;
		}
	}
	*/
	feature(in,features);
	
	switch (edition)
	{
		case EDITION_RMB1999:
			Digitfeature = Digitfeature1999;
			Charfeature = Charfeature1999;
			break;
		case EDITION_RMB2005:
			Digitfeature = Digitfeature2005;
			Charfeature = Charfeature2005;
			break;
		case EDITION_RMB2015:
			Digitfeature = Digitfeature2015;
			Charfeature = Charfeature2015;
			break;
		case EDITION_EUR:
			Digitfeature = DigitfeatureEUR;
			Charfeature = CharfeatureEUR;
			break;
		case EDITION_USD:
			Digitfeature = DigitfeatureUSD;
			Charfeature = CharfeatureUSD;
			break;
		case EDITION_JPY10000:
			Digitfeature = DigitfeatureJPY10000;
			Charfeature = CharfeatureJPY10000;
		case EDITION_JPY5000:
			Digitfeature = DigitfeatureJPY5000;
			Charfeature = CharfeatureJPY5000;
		case EDITION_JPY1000:
			Digitfeature = DigitfeatureJPY1000;
			Charfeature = CharfeatureJPY1000;
		case EDITION_GBP_OLD:
		case EDITION_GBP_NEW:
			Digitfeature = DigitfeatureGBP;
			Charfeature = CharfeatureGBP;
		
		case EDITION_HKD_CHAR_BIG:
			Digitfeature = DigitfeatureHKD_BIG;
			Charfeature  = CharfeatureHKD_BIG;
			break;
		case EDITION_HKD_CHAR_SMALL:
			Digitfeature = DigitfeatureHKD_SMALL;
			Charfeature = CharfeatureHKD_SMALL;
			break;
		case EDITION_HKD_CHAR_10:
			Digitfeature = DigitfeatureHKD_10;
			Charfeature = CharfeatureHKD_10;
			break;
		case EDITION_SGD:
			Digitfeature = DigitfeatureSGD;
			Charfeature = CharfeatureSGD;
			break;
		case EDITION_NTD:
			Digitfeature = DigitfeatureNTD;
			Charfeature = CharfeatureNTD;
		break;
	}

	switch (Type)
	{
		case 'd':
			temp = find_digit(features,Digitfeature);
			break;
		case 'c':
			temp = find_char(features,Charfeature);
			break;
		case 'm':
			temp = find_mixed(features,Digitfeature,Charfeature);
			break;
	}

	return temp;
}

//识别混合型
char find_mixed(short feature[],const short Digitfeature[][29],const short Charfeature[][29])
{
    int result_c[26],result_d[10];
    int i, j, c, d;
	int ww;
	//识别字母
    memset(result_c, 0, sizeof(unsigned int) * 26);
    for (i = 0; i < 26; i++)
        for (j = 0; j < 29; j++)
		{
			ww=(feature[j] - Charfeature[i][j]);
            result_c[i] += ww*ww;
		}
	//result_c[8] = 0x7fffffff;//将与I的距离设置为最大
	c = 0;
    for (i = 1; i < 26; i++)
        if (result_c[i] < result_c[c])
            c = i;
	//识别数字
	memset(result_d, 0, sizeof(unsigned int) * 10);
    for (i = 0; i < 10; i++)
        for (j = 0; j < 29; j++)
		{
			ww=(feature[j] - Digitfeature[i][j]);
            result_d[i] += ww*ww;
		}

    d = 0;
    for (i = 1; i < 10; i++)
        if (result_d[i] < result_d[d])
            d = i;
	if(result_c[c]<=result_d[d])
		return 'A'+c;
	else
		return '0'+d;
}

//识别数字
char find_digit(short feature[],const short Digitfeature[][29])
{
    int result_d[10];
    int i, j;
	int ww;
	
	memset(result_d, 0, sizeof(int) * 10);
    for (i = 0; i < 10; i++)
        for (j = 0; j < 29; j++)
		{
			ww=(feature[j] - Digitfeature[i][j]);
            result_d[i] += ww*ww;
		}
    j = 0;
    for (i = 1; i < 10; i++)
        if (result_d[i] < result_d[j])
            j = i;
    return '0' + j;
}

//识别字母
char find_char(short feature[],const short Charfeature[][29])
{
    int result_c[26];
    int i, j;
	int ww;
	
	memset(result_c, 0, sizeof(int) * 26);
    for (i = 0; i < 26; i++)
        for (j = 0; j < 29; j++)
		{
			ww=(feature[j] - Charfeature[i][j]);
            result_c[i] += ww*ww;
		}
	//result_c[8] = 0x7fffffff;//将与I的距离设置为最大
    j = 0;
    for (i = 1; i < 26; i++)
        if (result_c[i] < result_c[j])
            j = i;
    return 'A' + j;
}

char idenAAlpha (Byte *miniImage, int positionX[24], int positionY[24], Byte *thresh, 
				 int edition, int i, Byte * GZHImage)
{
	char tempchar;
	int average;
	if (positionX[(i<<1) + 1] < 0)
	{
		tempchar = '*';
		return tempchar;
	}

	//预先区分0、1
	average=positionX[(i<<1) + 1] - positionX[i<<1];
	//分割字符宽度
	if (average<=9)
	{
		tempchar = 'I';

		average = segmentChar(miniImage, singleChar, singleChar_Gray, positionX + (i<<1),
			positionY + (i<<1),thresh);
		addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);

		return tempchar;
	}
	average = segmentChar(miniImage, singleChar, singleChar_Gray, positionX + (i<<1), 
		positionY + (i<<1),thresh);//从灰度图中分割出字符并按25:18缩放并二值化
	addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);
	tempchar = ocrcore(singleChar,'c',edition);
	
	switch (tempchar)
	{
		case 'Q':
		case 'C':
		case 'G':
		case 'O':
		case 'D':
			tempchar = idenGCODandQ(singleChar,singleChar_Gray,average);
			if (tempchar == '?')
			{
				tempchar = idenOandD_Gray(singleChar,singleChar_Gray,average);
			}
			break;
		case 'F':
		case 'P':
			tempchar = idenFandP(singleChar);
			break;
		case 'M':
		case 'N':
			tempchar = idenMandN(singleChar);
			break;
	}
	switch(tempchar)
	{
		case 'S':
		case 'G':
			tempchar = idenSandG(singleChar,singleChar_Gray,average);
			break;
	}
	return tempchar;
}

char idenADigit (Byte *miniImage, int positionX[24], int positionY[24], Byte *thresh, 
				 int edition, int i, Byte * GZHImage)
{
	char tempchar;
	int average;
	if (positionX[(i<<1) + 1] < 0)
	{
		tempchar = '*';
		//addASpaceToGZHImage (GZHImage, i);
		return tempchar;
	}

	//预先区分0、1
	average=positionX[(i<<1) + 1] - positionX[i<<1];
	//字符宽度
	if (average<=9)
	{
		tempchar = '1';

		average = (16 - average) / 2;
		positionX[(i<<1) + 1] += average;
		positionX[i<<1] -= average;

		average = segmentChar(miniImage, singleChar, singleChar_Gray, positionX + (i<<1), 
			positionY + (i<<1),thresh);
		addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);

		return tempchar;
	}
	//从灰度图中分割出字符并按25:18缩放并二值化
	average = segmentChar(miniImage, singleChar, singleChar_Gray, positionX + (i<<1),
		positionY + (i<<1),thresh);
	addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);
	tempchar = ocrcore(singleChar,'d',edition);

	//考虑灰度灰度信息区别8、6、9
	switch (tempchar)
	{
		case '9':
		case '5':
		case '8':
		case '6':
			tempchar = idenGrayTh(singleChar_Gray,edition, tempchar, average);
			break;
		case '7':
			if (positionX[(i<<1) + 1] - positionX[i<<1] < 14)
				tempchar = '1';
			break; 
	}
	
	return tempchar;
}

char idenAMixed (Byte *miniImage, int positionX[24], int positionY[24], Byte *thresh, 
				 int edition, int i, Byte * GZHImage)
{
	char tempchar;
	int average;
	if (positionX[(i<<1) + 1] < 0)
	{
		tempchar = '*';
		//addASpaceToGZHImage (GZHImage, i);
		return tempchar;
	}

	//预先区分0、1
	average=positionX[(i<<1) + 1] - positionX[i<<1];
	//分割字符宽度
	if (average<=9)
	{
		tempchar = 'I';

		average = (16 - average) / 2;
		positionX[(i<<1) + 1] += average;
		positionX[i<<1] -= average;

		average = segmentChar(miniImage, singleChar, singleChar_Gray, positionX + (i<<1),
			positionY + (i<<1),thresh);
		addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);

		return tempchar;
	}
	average = segmentChar(miniImage, singleChar, singleChar_Gray, positionX + (i<<1), 
		positionY + (i<<1),thresh);//从灰度图中分割出字符并按25:18缩放并二值化

	addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);

	tempchar = ocrcore(singleChar,'m',edition);
	
	switch (tempchar)
	{
		case '9':
		case '5':
		case '6':
		case '8':
		case 'B':
			tempchar = idenGrayTh(singleChar_Gray,edition, tempchar, average);	
			if (tempchar == '8')
				tempchar = idenBand8 (singleChar_Gray);
			break;
		case '0':
			tempchar = 'O';
	}
	
	switch (tempchar)
	{
		case 'Q':
		case 'C':
		case 'G':
		case 'O':
		case 'D':
			tempchar = idenGCODandQ(singleChar,singleChar_Gray,average);
			if (tempchar == '?')
			{
				tempchar = idenOandD_Gray(singleChar,singleChar_Gray,average);
			}
			break;
		case 'S':
		case '5':
			tempchar = idenSand5(singleChar_Gray);
			break;
		case 'F':
		case 'P':
			tempchar = idenFandP(singleChar);
			break;
		case 'M':
		case 'N':
			tempchar = idenMandN(singleChar);
			break;
	}
	return tempchar;
}

int idenRMB05(Byte *miniImage, int positionX[24], int positionY[24], Byte *thresh, 
			  char * result_all, Byte * GZHImage, int charNums)
{
	int i;
	int CertainChar = 1;
	
	//识别前1个字符

	result_all[0] = idenAAlpha (miniImage, positionX, positionY, thresh, 
		EDITION_RMB2005, 0, GZHImage);

	for ( i = 1; i < 4; i ++)
	{
		if (CertainChar == 2)
        	break;
		result_all[i] = idenAMixed (miniImage, positionX, positionY, thresh, 
			EDITION_RMB2005, i, GZHImage);
        switch (result_all[i])
        {
            case '0':
            case '1':
            case 'O':
            case 'I':
                break;
            case 'B':
            case 'D':
                break;
            case '8':
                break;
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '9':
                break;
            default:
                CertainChar ++;
                break;
        }	
	}
	
	//识别后面6~8个数字
	for (; i < charNums; i++)
    {
		result_all[i] = idenADigit (miniImage, positionX, positionY, thresh, 
			EDITION_RMB2005, i, GZHImage);
	}

	postProcess (result_all);

	for (; i < 10; i ++)
	{
		result_all[i] = '*';
		addASpaceToGZHImage ((Byte*)(pRecord.ImageSNo.SNo), i);
	}

	//识别成功
	return 2;
}

//1元RMB专用
int idenRMB1RMB(Byte *miniImage, int positionX[24], int positionY[24], Byte *thresh, 
			char * result_all, Byte * GZHImage, int charNums)
{
	int i;
	int CertainChar = 0;
	
	//识别前1个字符

	result_all[0] = idenAAlpha (miniImage, positionX, positionY, thresh, 
		EDITION_RMB1999, 0, GZHImage);

	for ( i = 1; i < 4; i ++)
	{
		if (CertainChar == 2)
        	break;
	
		result_all[i] = idenAMixed (miniImage, positionX, positionY, thresh, 
			EDITION_RMB2005, i, GZHImage);
		switch (result_all[i])
        {
            case '0':
            case '1':
            case 'O':
            case 'I':
                break;
            case 'B':
            case 'D':
                break;
            case '8':
                break;
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '9':
                break;
            default:
                CertainChar ++;
                break;
        }	
	}
	
	//识别后面6~8个数字
	for (; i < charNums; i++)
    {
		result_all[i] = idenADigit (miniImage, positionX, positionY, thresh, 
			EDITION_RMB1999, i, GZHImage);
	}

	postProcess (result_all);

	for (; i < 10; i ++)
	{
		result_all[i] = '*';
		addASpaceToGZHImage ((Byte*)(pRecord.ImageSNo.SNo), i);
	}

	//识别成功
	return 2;
}

int idenRMB99(Byte *miniImage, int positionX[24], int positionY[24], 
			  Byte *thresh, char * result_all, Byte * GZHImage, int charNums)
{
	int i;
	//char status[3];
	
	//识别前2个字符

	for (i = 0; i < 2; i ++)
	{
		result_all[i] = idenAAlpha (miniImage, positionX, positionY, thresh, 
			EDITION_RMB1999, i, GZHImage);
	}
	//识别后面8个数字
	for (; i < charNums; i++)
    {
		result_all[i] = idenADigit (miniImage, positionX, positionY, thresh, 
			EDITION_RMB1999, i, GZHImage);
	}
	for (; i < 10; i ++)
	{
		result_all[i] = '*';
		addASpaceToGZHImage ((Byte*)(pRecord.ImageSNo.SNo), i);
	}

	//识别成功
	return 2;
}


int idenRMB15(Byte *miniImage, int positionX[24], int positionY[24], Byte *thresh, 
			  char * result_all, Byte * GZHImage, int charNums)
{
	int i;
	int average;
	for (i = 0; i < 2; i ++)
	{
	 if (positionX[(i<<1) + 1] < 0)
	    {
		result_all[i] = '*';
		}

		//预先区分0、1
		average=positionX[(i<<1) + 1] - positionX[i<<1];
		//分割字符宽度
		if (average<=9)
		{
			result_all[i] = 'I';

			average = segmentChar(miniImage, singleChar, singleChar_Gray, 
				positionX + (i<<1), positionY + (i<<1),thresh);
			addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);
		}
		//从灰度图中分割出字符并按25:18缩放并二值化
		average = segmentChar(miniImage, singleChar, singleChar_Gray, 
			positionX + (i<<1), positionY + (i<<1),thresh);
		addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);
		result_all[i] = ocrcore(singleChar,'c',EDITION_RMB2015);
	}

	for (; i < charNums; i++)
	{
		if (positionX[(i<<1) + 1] < 0)
		{
			result_all[i] = '*';
			//addASpaceToGZHImage (GZHImage, i);
		}

	//预先区分0、1
		average=positionX[(i<<1) + 1] - positionX[i<<1];
		//字符宽度
		if (average<=9)
		{
			result_all[i] = '1';

			average = (16 - average) / 2;
			positionX[(i<<1) + 1] += average;
			positionX[i<<1] -= average;

			average = segmentChar(miniImage, singleChar, singleChar_Gray, 
				positionX + (i<<1), positionY + (i<<1),thresh);
			addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);
		}
		//从灰度图中分割出字符并按25:18缩放并二值化
		average = segmentChar(miniImage, singleChar, singleChar_Gray, 
			positionX + (i<<1), positionY + (i<<1),thresh);
		addADigitToGZHImage (singleChar, (Byte*)(pRecord.ImageSNo.SNo), i);
		result_all[i] = ocrcore(singleChar,'d',EDITION_RMB2015);
	}
	for (; i < 10; i ++)
	{
		result_all[i] = '*';
		addASpaceToGZHImage ((Byte*)(pRecord.ImageSNo.SNo), i);
	}
	return 2;
}
