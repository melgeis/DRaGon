from PIL import Image
from scipy.constants import speed_of_light
import requests
import os
import numpy as np
import random
import math
import uuid



def eps2png(args, city, size, folder, file, folder_ending='') -> bool:
    if file.endswith(".eps"):
        im = Image.open(file)
        try:
            im = im.resize((size,size))
        except:
            print(file)
            return False

        if city == 'do':
            for area in args.areas:
                if '/'+area+'/' in file:
                    im.save(args.base_path + 'dataset_processed' + folder_ending + '/dortmund/' + str(size) + folder + area + '/' + file.split('/')[-1].split('.')[0] + '.png')
                    return True
        
        elif city == 'aa':
            for month in args.folders:
                if '/'+month+'/' in file:
                    for height in args.heights:
                        if '/'+month+'/'+height+'/' in file:
                            im.save(args.base_path + 'dataset_processed/aarhus/' + str(size) + folder + month + '_' + height + '/' + file.split('/')[-1].split('.')[0] + '.png')
                            return True

        elif city == 'ko':
            im.save(args.base_path + 'dataset_processed/kopenhagen/' + str(size) + folder + file.split('/')[-1].split('.')[0] + '.png')
            return True

        elif city == 'wu':
            im.save(args.base_path + 'dataset_processed/wuppertal/' + str(size) + folder + file.split('/')[-1].split('.')[0] + '.png')
            return True

        elif city == 'REM':
            _path = file.split('engineered')[0] + 'processed' + folder + file.split('/')[-1].split('.')[0] + '.png'
            im.save(_path)
            return True

    return False


# UMa LOS
def UMa_LOS_pathloss(d_2D, d_3D, f_Hz, h_UT=1.5, h_BS=25):
    f_GHz = f_Hz/pow(10,9)
    d_BP = 4 * h_BS * h_UT * f_Hz/speed_of_light
    PL_LOS = np.zeros(len(d_2D))

    for i in range(len(d_2D)):
        if d_2D[i] <= d_BP:
            PL_LOS[i] += 28 + 22*np.log10(d_3D[i]) + 20*np.log10(f_GHz)
        else:
            PL_LOS[i] += 28 + 40*np.log10(d_3D[i]) + 20*np.log10(f_GHz) - 9*np.log10(pow(d_BP, 2) + pow(h_BS - h_UT,2))

    return PL_LOS


# UMa B NLOS
def UMa_B_NLOS_pathloss(d_2D, d_3D, f_Hz, h_UT=1.5, h_BS=30):
    f_GHz = f_Hz/pow(10,9)
    PL_NLOS = 13.54 + 39.08*np.log10(d_3D) + 20*np.log10(f_GHz) - 0.6*(h_UT - 1.5)
    PL_LOS = UMa_LOS_pathloss(d_2D, d_3D, f_Hz, h_UT, h_BS)
    return np.maximum(PL_NLOS, PL_LOS)


# compute cartesian distance 
def compute_distance(_lat: float, _lon: float, _ref_lat: float, _ref_lon: float) -> float:    
    pi = 3.141592654
    dlat = (_lat - _ref_lat) / 180 * pi
    dlon = (_lon - _ref_lon) / 180 * pi

    a = pow(math.sin(dlat/2), 2) + math.cos(_lat / 180 * pi) * math.cos(_ref_lat / 180 * pi) * pow(math.sin(dlon/2), 2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))

    return 6367 * c * 1000


# RSRP reference basend on bandwidth
def get_rsrp_ref(B_MHz):
    if B_MHz == 1.4:
        RB = 6
    else:
        RB = B_MHz / 0.2

    return 10*np.log10(12*RB)

