import json
import os
from shutil import copyfile

import openpyxl
import EB_AFIT_core as core

dict_packed = {}

copyfile('box.txt', 'boxlist.txt')

def not_packed(json_data):
    f = open('boxlist.txt', 'w')
    a = []

    a.append(json_data['pallet']['pallet_x'])
    a.append(json_data['pallet']['pallet_y'])
    a.append(json_data['pallet']['pallet_z'])

    f.write(str(a).replace('[', '').replace(']', '') + '\n')

    for i in json_data['not_packed']:

        #f.write(str(i).replace('[', '').replace(']', '').replace('\'', '').replace(' ', '') + '\n' )
        if not i:
            break
        qwrt = str(i[0]).replace(' ', '')
        f.write(f'{qwrt} {i[1]}, {i[2]}, {i[3]}, {i[4]}, 1' + '\n')

    f.close()

    return 0


def boxes(json_data):
    f = open('boxes.txt', 'w')
    a = []
    b = []

    a.append(json_data['pallet']['pallet_x'])
    a.append(json_data['pallet']['pallet_y'])
    a.append(json_data['pallet']['pallet_z'])

    dict_packed['pallet'] = a

    for i in json_data['boxes']:
        b.append(i)

    dict_packed['item'] = b
    f.write(str(dict_packed).replace('\'', '"').replace(' ', '') + '\n' + ",[]")
    f.close()

    return 0

def excel(t):
    print(json.dumps(templates, sort_keys=False, indent=4))
    for i in templates['boxes']:
        print(i)
    wb = openpyxl.load_workbook(filename = 'xl.xlsx')
    sheet = wb['test']
    w = 1
    for rew in templates['pallet']:
        for k in rew:
            sheet.cell(row=2, column=w).value = k
            w += 1
    i = 1
    q = 1
    for rec in templates['boxes']:
        if not rec:
            i += 1
        for j in rec:
            sheet.cell(row=5 + i, column=q).value = j

            q += 1
        i += 1
        q = 1
    # сохраняем данные
    name = 'report/openpyxl'+ str(t) + '.xlsx'
    wb.save(name)
    wb.close()

filename =  'boxlist.txt'

filename_with_path = os.path.abspath(os.path.join(os.path.dirname(__file__), filename))
core.calc(filename_with_path)



with open('.~3d') as f:
    templates = json.load(f)
not_packed(templates)
boxes(templates)


# Если неупакованных коробок нет

t = 1
while len(templates['not_packed']) >1 :
    print("vse taki gei")
    # отправка названия файла в код МаксаН
   # filename1 =  'boxlist.txt'
    with open('.~3d') as f:
        templates = json.load(f)

    filename_with_path = os.path.abspath(os.path.join(os.path.dirname(__file__), filename))
    core.calc(filename_with_path)
    not_packed(templates)
    boxes(templates)
    excel(t)
    t += 1

os.remove("boxes.txt")
os.remove("boxlist.txt")
os.remove(".~3d")
