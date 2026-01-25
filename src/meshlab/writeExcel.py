# -*- coding: utf-8 -*-
"""
Created on Sat Jan 22 17:21:26 2022

@author: MSI-NB
"""

import sys
import xlsxwriter


# Create an new Excel file and add a worksheet.
workbook  = xlsxwriter.Workbook('demo.xlsx')
worksheet = workbook.add_worksheet()

# Widen the first column to make the text clearer.
worksheet.set_column('A:I', 7)
worksheet.set_column('D:D', 0)
worksheet.set_column('B:B', 15)
worksheet.set_column('E:E', 15)
worksheet.set_column('H:I', 10.2)
worksheet.set_row(0,50)

'''
# Insert an image.
worksheet.merge_range('B3:D4',    'Merged Cells')
worksheet.insert_image('B4', 'logo.png')
'''

#标题
cellFormat0 = workbook.add_format()
cellFormat0.set_bg_color('#133C99')  
cellFormat0.set_font_name('calibri')
cellFormat0.set_font_size(30)
cellFormat0.set_font_color('white')
cellFormat0.set_border(1)
cellFormat0.set_bold()
cellFormat0.set_align('center')
cellFormat0.set_align('vcenter')

#数据表格头
cellFormat1 = workbook.add_format()
cellFormat1.set_bg_color('#133C99')  ##4472C4
cellFormat1.set_font_name('calibri')
cellFormat1.set_font_size(11)
cellFormat1.set_font_color('white')
# cellFormat1.set_border(1)

#数据表格头2
cellFormat2 = workbook.add_format()
cellFormat2.set_bg_color('#4472C4')
# cellFormat2.set_border(1)

#数据表格内容 
cellFormat3 = workbook.add_format()
cellFormat3.set_font_name('calibri')
cellFormat3.set_font_size(10)
cellFormat3.set_num_format('0.0')
cellFormat3.set_align('left')

#数据表格内容 颅骨比例
cellFormat4 = workbook.add_format()
cellFormat4.set_font_name('calibri')
cellFormat4.set_font_size(10)
cellFormat4.set_num_format('0.000')
cellFormat4.set_align('left')

#数据表格内容 
cellFormat5 = workbook.add_format()
cellFormat5.set_font_name('calibri')
cellFormat5.set_font_size(10)
cellFormat5.set_num_format('yy/mm/dd')

info = ['编号','姓名','生日','报告日期','疗程','备注']

measurements = ['Circumference','Cranial Breadth (M-L)','Cranial Length (A-P)','Cephalic Ratio (M-L/A-P)',
                'Radial Symmetry Index (RSI)','Oblique-Diagonal 1 at -30 degree (D1)','Oblique-Diagonal 2 at 30 degree (D2)','Cranial Vault Asymmetry Index (CVAI)'];

measurementsC = ['头围','颅宽','颅长','颅骨比例','放射对称指数','对角线1 -30度','对角线2 30度','颅顶不对称指数']

volumes = ['Q1 Volume (A/L)','Q2 Volume (A/R)','Q3 Volume (P/R)','Q4 Volume (P/L)','Anterior Symmetry Ratio (ASR)',
           'Posterior Symmetry Ratio (PSR)','Overall Symmetry Ratio','Anterior-Posterior Volume Ratio']

volumesC = ['颅脑容积第一象限','颅脑容积第二象限','颅脑容积第三象限','颅脑容积第四象限','前颅对称指数','后颅对称指数','总对称指数','颅脑容积前后比例']

#read infomation
dataMeasureLevel3 = {}
dataMeasureLevel5 = {}
dataMeasureLevel8 = {}
dataVolume        = {}
inputFile = open("temp.data",'r')
lines = inputFile.readlines();
dataRead = dataMeasureLevel3;
for line in lines:
    line  = line[:-1]
    data = line.split('#')
    if data[0] == 'Level':
        if data[1] == '3':
            dataRead = dataMeasureLevel3
        elif data[1] == '5':
            dataRead = dataMeasureLevel5
       
#title
        elif data[1] == '8':
            dataRead = dataMeasureLevel8
    elif data[0] == 'Volume':
        dataRead = dataVolume
    else:
        dataRead[data[0]] = float(data[1])
worksheet.merge_range('A1:I1','云量几何婴儿头型矫正',cellFormat0)

#info
for i in range(1,7):
    # worksheet.merge_range(i, 1, i, 5,'')
    worksheet.write(i,0,info[i-1],cellFormat3)
worksheet.insert_image('D2', 'image plane.png', {'x_scale': 0.13, 'y_scale': 0.13 * 1.26, 'y_offset': 0.8})
worksheet.write_formula('B5', '= NOW()', cellFormat5)

#Level 3
worksheet.merge_range('A8:F8','Level(3) Measurements', cellFormat1)
worksheet.write('H8', '', cellFormat2)
worksheet.write_formula('G8', '= CONCATENATE(CEILING((NOW()-B4)/7,1),"周")', cellFormat1)
worksheet.write('I8', '', cellFormat2)

count = 0
for i in range(8,16):
    worksheet.write(i,0,measurements[count],cellFormat3)
    worksheet.write(i,4,measurementsC[count],cellFormat3)
    if count == 3:
        worksheet.write(i,6,dataMeasureLevel3[measurements[count]],cellFormat4)
    elif count == 7:
        worksheet.write(i,6,dataMeasureLevel3[measurements[count]] * 100, cellFormat3)
        worksheet.write(i,5,'%',cellFormat3)
    else:
        worksheet.write(i,6,dataMeasureLevel3[measurements[count]], cellFormat3)
        worksheet.write(i,5,'(mm)',cellFormat3)
    count += 1; 
 #save images
worksheet.insert_image('H9', 'image3.png', {'x_scale': 0.619, 'y_scale': 0.62 * 1.2, 'y_offset': 0.8})

#Level 5
worksheet.merge_range('A17:F17','Level(5) Measurements', cellFormat1)
worksheet.merge_range('G17:I17','', cellFormat2)
count = 0
for i in range(17,25):
    worksheet.write(i,0,measurements[count],cellFormat3)
    worksheet.write(i,4,measurementsC[count],cellFormat3)
    if count == 3:
        worksheet.write(i,6,dataMeasureLevel5[measurements[count]],cellFormat4)
    elif count == 7:        
        worksheet.write(i,6,dataMeasureLevel5[measurements[count]] * 100, cellFormat3)
        worksheet.write(i,5,'%',cellFormat3)
    else:
        worksheet.write(i,6,dataMeasureLevel5[measurements[count]], cellFormat3)
        worksheet.write(i,5,'(mm)',cellFormat3)
    count += 1;
 #save images
# worksheet.merge_range('H18:I25','')
worksheet.insert_image('H18', 'image5.png',{'x_scale': 0.619, 'y_scale': 0.62 * 1.2, 'y_offset': 0.8})

#Level 8
worksheet.merge_range('A26:F26','Level(8) Measurements', cellFormat1)
worksheet.merge_range('G26:I26','', cellFormat2)
count = 0
for i in range(26,34):
    worksheet.write(i,0,measurements[count],cellFormat3)
    worksheet.write(i,4,measurementsC[count],cellFormat3)
    if count == 3:
        worksheet.write(i,6,dataMeasureLevel8[measurements[count]],cellFormat4)
    elif count == 7:
        worksheet.write(i,6,dataMeasureLevel8[measurements[count]] * 100, cellFormat3)
        worksheet.write(i,5,'%',cellFormat3)
    else:
        worksheet.write(i,6,dataMeasureLevel8[measurements[count]], cellFormat3)
        worksheet.write(i,5,'(mm)',cellFormat3)
    count += 1;    
 #save images
worksheet.insert_image('H27', 'image8.png', {'x_scale': 0.619, 'y_scale': 0.62 * 1.2, 'y_offset': 0.8})

#Volumes
worksheet.merge_range('A35:F35','Volumes(Level 2 to Level 8)', cellFormat1)
worksheet.merge_range('G35:I35','', cellFormat2)
count = 0
for i in range(35,43):
    worksheet.write(i,0,volumes[count],cellFormat3)
    worksheet.write(i,4,volumesC[count],cellFormat3)
    # cellFormat3.set_num_format('0.0')
    if count < 4:
        worksheet.write(i,5,'(cc)',cellFormat3)
        worksheet.write(i,6,dataVolume[volumes[count]],cellFormat3)
    else:
        worksheet.write(i,5,'%',cellFormat3)
        worksheet.write(i,6,dataVolume[volumes[count]] * 100,cellFormat3)
    count += 1;  

worksheet.insert_image('H36', 'image split.png', {'x_scale': 0.219, 'y_scale': 0.180 * 1.26, 'y_offset': 0.8})

worksheet.merge_range('A44:F44','', cellFormat1)
worksheet.merge_range('G44:I44','', cellFormat2)
    
workbook.close()

import os
if len(sys.argv) > 1:
    dst = sys.argv[1]
    if os.path.isdir(dst):
        import shutil
        print('copy to destination')
        shutil.copyfile('demo.xlsx', dst + '/result.xlsx')
