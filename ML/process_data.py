import os
import json
import threading
import csv
import uuid
import time
import queue 

import pandas as pd
import numpy as np

from progress.bar import IncrementalBar
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

from helpers_ML import *
import init_model


class TaskQueue(queue.Queue):

    def __init__(self, num_workers=1):
        queue.Queue.__init__(self)
        self.num_workers = num_workers

    def add_task(self, task, *args, **kwargs):
        args = args or ()
        kwargs = kwargs or {}
        self.put((task, args, kwargs))

    def start_workers(self):
        for i in range(self.num_workers):
            t = threading.Thread(target=self.worker, args=(i,))
            t.daemon = True
            t.start()

    def worker(self, id):
        while True:
            tmp = self.get()[0]
            eps2png(tmp[0], tmp[1], tmp[3], tmp[4], tmp[2], tmp[-1])
            self.task_done()



def getCell(cell_size_m: float, xdist: float, ydist: float):
    col = int(ydist / cell_size_m)
    row = int(xdist / cell_size_m)
    return row, col


def adapt_pickle_REM(city, range=25, offset_per_cell=False):
    if city == 'do':
        for area in args.areas:
            if offset_per_cell:
                path = args.base_path + '/dataset_processed/dortmund/' + area + '_offset_per_cell.pkl'
            else:
                path = args.base_path + '/dataset_processed/dortmund/' + area + '.pkl'

            features = pd.read_pickle(path)
            features = features.astype({'RSRP': float})

            bar = IncrementalBar('read features/' + area, max = len(features))
            cur_id = '0'

            _tmp = []
            _idx = []

            for index, feature in features.iterrows():
                id = str(feature['id'])[:-4]

                if id != cur_id:
                    with open(args.base_path + 'REM/Dortmund/' + area + '/' + id + '_' + str(range) + '.csv', 'r') as f:
                        reader = csv.reader(f)
                        tmp = list(reader)
                        ref_lat, ref_lon, cell_size_m = [float(x) for x in tmp[1]]
                        data = np.array(tmp[2::])
                        cur_id = id

                y = compute_distance(feature['lat'], ref_lon, ref_lat, ref_lon)
                x = compute_distance(ref_lat, feature['lon'], ref_lat, ref_lon)

                col, row = getCell(cell_size_m, x, y)

                _cur = [id, features.loc[index].lat, features.loc[index].lon]

                if _cur not in _tmp:
                    _tmp.append(_cur)
                    features.at[index,'RSRP'] = data[row][col]
                else:
                    _idx.append(index)

                bar.next()

            features.drop(_idx, inplace=True)

            # save dataframe as pickle
            if offset_per_cell:
                features.to_pickle(args.base_path + 'dataset_processed/dortmund/' + area + '_offset_per_cell_REM_' + str(range) + '.pkl')
            else:
                features.to_pickle(args.base_path + 'dataset_processed/dortmund/' + area + '_REM_' + str(range) + '.pkl')
            print('\n saved ' + area + ' as pkl')

    elif city == 'ko':
        if offset_per_cell:
            path = args.base_path + '/dataset_processed/kopenhagen/kopenhagen_offset_per_cell.pkl'
        else:
            path = args.base_path + '/dataset_processed/kopenhagen/kopenhagen.pkl'

        features = pd.read_pickle(path)
        features = features.astype({'RSRP': float})

        bar = IncrementalBar('read features kopenhagen', max = len(features))
        cur_id = '0'

        _tmp = []
        _idx = []

        for index, feature in features.iterrows():
            id = str(feature['id'])[:-5]

            if id != cur_id:
                with open(args.base_path + 'REM/Kopenhagen/' + id + '_' + str(range) + '.csv', 'r') as f:
                    reader = csv.reader(f)
                    tmp = list(reader)
                    ref_lat, ref_lon, cell_size_m = [float(x) for x in tmp[1]]
                    data = np.array(tmp[2::])
                    cur_id = id

            y = compute_distance(feature['lat'], ref_lon, ref_lat, ref_lon)
            x = compute_distance(ref_lat, feature['lon'], ref_lat, ref_lon)

            col, row = getCell(cell_size_m, x, y)

            _cur = [id, features.loc[index].lat, features.loc[index].lon]

            if _cur not in _tmp:
                _tmp.append(_cur)
                features.at[index,'RSRP'] = data[row][col]
            else:
                _idx.append(index)

            bar.next()

        features.drop(_idx, inplace=True)

        # save dataframe as pickle
        if offset_per_cell:
            features.to_pickle(args.base_path + 'dataset_processed/kopenhagen/kopenhagen_offset_per_cell_REM_' + str(range) + '.pkl')
        else:
            features.to_pickle(args.base_path + 'dataset_processed/kopenhagen/kopenhagen_REM_' + str(range) + '.pkl')
        print('\n saved kopenhagen as pkl')

    elif city == 'wu':
        if offset_per_cell:
            path = args.base_path + '/dataset_processed/wuppertal/wuppertal_offset_per_cell.pkl'
        else:
            path = args.base_path + '/dataset_processed/wuppertal/wuppertal.pkl'

        features = pd.read_pickle(path)
        features = features.astype({'RSRP': float})

        bar = IncrementalBar('read features wuppertal', max = len(features))
        cur_id = '0'

        _tmp = []
        _idx = []

        for index, feature in features.iterrows():
            id = str(feature['id'])[:-4]

            if id != cur_id:
                with open(args.base_path + 'REM/Wuppertal/' + id + '_' + str(range) + '.csv', 'r') as f:
                    reader = csv.reader(f)
                    tmp = list(reader)
                    ref_lat, ref_lon, cell_size_m = [float(x) for x in tmp[1]]
                    data = np.array(tmp[2::])
                    cur_id = id

            y = compute_distance(feature['lat'], ref_lon, ref_lat, ref_lon)
            x = compute_distance(ref_lat, feature['lon'], ref_lat, ref_lon)

            col, row = getCell(cell_size_m, x, y)

            _cur = [id, features.loc[index].lat, features.loc[index].lon]

            if _cur not in _tmp:
                _tmp.append(_cur)
                features.at[index,'RSRP'] = data[row][col]
            else:
                _idx.append(index)

            bar.next()

        features.drop(_idx, inplace=True)

        # save dataframe as pickle
        if offset_per_cell:
            features.to_pickle(args.base_path + 'dataset_processed/wuppertal/wuppertal_offset_per_cell_REM_' + str(range) + '.pkl')
        else:
            features.to_pickle(args.base_path + 'dataset_processed/wuppertal/wuppertal_REM_' + str(range) + '.pkl')
        print('\n saved wuppertal as pkl')

    elif city == 'aa':
        for month in args.folders:
            for height in args.heights:
                if offset_per_cell:
                    path = args.base_path + '/dataset_processed/aarhus/' + month + '_' + height + '_offset_per_cell.pkl'
                else:
                    path = args.base_path + '/dataset_processed/aarhus/' + month + '_' + height + '.pkl'

                if not os.path.exists(path):
                    continue
                
                features = pd.read_pickle(path)
                features = features.astype({'RSRP': float})

                bar = IncrementalBar('read features/' + month + '_' + height, max = len(features))
                cur_id = '0'

                _tmp = []
                _idx = []

                for index, feature in features.iterrows():
                    id = str(feature['id'])[:-4]

                    if id != cur_id:
                        with open(args.base_path + 'REM/Aarhus/' + month + '/' + height + '/' + id + '_' + str(range) + '.csv', 'r') as f:
                            reader = csv.reader(f)
                            tmp = list(reader)
                            ref_lat, ref_lon, cell_size_m = [float(x) for x in tmp[1]]
                            data = np.array(tmp[2::])
                            cur_id = id

                    y = compute_distance(feature['lat'], ref_lon, ref_lat, ref_lon)
                    x = compute_distance(ref_lat, feature['lon'], ref_lat, ref_lon)

                    col, row = getCell(cell_size_m, x, y)

                    _cur = [id, features.loc[index].lat, features.loc[index].lon]

                    if _cur not in _tmp:
                        _tmp.append(_cur)
                        features.at[index,'RSRP'] = data[row][col]
                    else:
                        _idx.append(index)

                    bar.next()

                features.drop(_idx, inplace=True)

                # save dataframe as pickle
                if offset_per_cell:
                    features.to_pickle(args.base_path + 'dataset_processed/aarhus/' + month + '_' + height + '_offset_per_cell_REM_' + str(range) + '.pkl')
                else:
                    features.to_pickle(args.base_path + 'dataset_processed/aarhus/' + month + '_' + height + '_REM_' + str(range) + '.pkl')
                print('\n saved ' + month + '_' + height + ' as pkl')


def adapt_pickle_offset(city):
    if city == 'do':
        for area in args.areas:
            path = args.base_path + 'dataset_processed/dortmund/' + area + '.pkl'
            features = pd.read_pickle(path)
            
            with open(args.base_path + 'analyzed_measurements/Dortmund/offset_Prx_per_cell.csv', 'r') as f:
                reader = csv.reader(f)
                tmp = list(reader)[1::]

            offset = dict()
            for elem in tmp:
                offset[elem[0]] = float(elem[1])

            idx = []

            for index, feature in features.iterrows():
                id = str(feature['id'])[:-4]
                if id in offset:
                    features.at[index,'offset'] = offset[id]
                else:
                    idx.append(index)

            features.drop(index=idx, inplace=True)

            # save dataframe as pickle
            features.to_pickle(args.base_path + 'dataset_processed/dortmund/' + area + '_offset_per_cell.pkl')
            print('\n saved ' + area + ' as pkl')

    elif city == 'ko':
        path = args.base_path + '/dataset_processed/kopenhagen/kopenhagen.pkl'
        features = pd.read_pickle(path)
        
        with open(args.base_path + 'analyzed_measurements/Kopenhagen/offset_Prx_per_cell.csv', 'r') as f:
            reader = csv.reader(f)
            tmp = list(reader)[1::]

        offset = dict()
        for elem in tmp:
            offset[elem[0]] = float(elem[1])

        idx = []

        for index, feature in features.iterrows():
            id = str(feature['id'])[:-5]
            if id in offset:
                features.at[index,'offset'] = offset[id]
            else:
                idx.append(index)

        features.drop(index=idx, inplace=True)

        # save dataframe as pickle
        features.to_pickle(args.base_path + 'dataset_processed/kopenhagen/kopenhagen_offset_per_cell.pkl')
        print('\n saved kopenhagen as pkl')

    elif city == 'wu':
        path = args.base_path + '/dataset_processed/wuppertal/wuppertal.pkl'
        features = pd.read_pickle(path)
        
        with open(args.base_path + 'analyzed_measurements/Wuppertal/offset_Prx_per_cell.csv', 'r') as f:
            reader = csv.reader(f)
            tmp = list(reader)[1::]

        offset = dict()
        for elem in tmp:
            offset[elem[0]] = float(elem[1])

        idx = []

        for index, feature in features.iterrows():
            id = str(feature['id'])[:-4]
            if id in offset:
                features.at[index,'offset'] = offset[id]
            else:
                idx.append(index)

        features.drop(index=idx, inplace=True)

        # save dataframe as pickle
        features.to_pickle(args.base_path + 'dataset_processed/wuppertal/wuppertal_offset_per_cell.pkl')
        print('\n saved wuppertal as pkl')

    elif city == 'aa':
        for month in args.folders:
            for height in args.heights:
                path = args.base_path + '/dataset_processed/aarhus/' + month + '_' + height + '.pkl'
                if not os.path.exists(path):
                    continue
                features = pd.read_pickle(path)

                with open(args.base_path + 'analyzed_measurements/Aarhus/' + month + '_' + height + '_offset_Prx_per_cell.csv', 'r') as f:
                    reader = csv.reader(f)
                    tmp = list(reader)[1::]

                offset = dict()
                for elem in tmp:
                    offset[elem[0]] = float(elem[1])

                idx = []

                for index, feature in features.iterrows():
                    id = str(feature['id'])[:-4]
                    if id in offset:
                        features.at[index,'offset'] = offset[id]
                    else:
                        idx.append(index)

                features.drop(index=idx, inplace=True)

                # save dataframe as pickle
                features.to_pickle(args.base_path + 'dataset_processed/aarhus/' + month + '_' + height + '_offset_per_cell.pkl')
                print('\n saved ' + month + '_' +  height + ' as pkl')


def generate_pickle_REM(path, offset_per_cell=False):
    # read json files and generate pickle file
    print("generate pickle files")

    bar = IncrementalBar('read features' , max = len(os.listdir(path)))

    if not offset_per_cell:
        for folder in os.listdir(path):
            if os.path.isdir(path + folder):
                _file = path + folder + '/features/feature'
                file = path + folder + '/features/feature'
                try:
                    features = pd.read_csv(_file + '.csv')
                    # save dataframe as pickle
                    features.to_pickle(file + '.pkl')
                except FileNotFoundError:
                    print(file + '.csv does not exist')
        
            bar.next()

    else:
        for folder in os.listdir(path):
            if os.path.isdir(path + folder):
                file = path + folder + '/features/feature'
                try:
                    features = pd.read_pickle(file + '.pkl')
                except FileNotFoundError:
                    features = pd.read_csv(file + '.csv')
                    features.to_pickle(file + '.pkl')
        
                with open(args.base_path + 'analyzed_measurements/Dortmund/offset_Prx_per_cell.csv', 'r') as f:
                    reader = csv.reader(f)
                    tmp = list(reader)[1::]

                offset = dict()
                for elem in tmp:
                    offset[elem[0]] = float(elem[1])

                idx = []

                for index, feature in features.iterrows():
                    if folder in offset:
                        features.at[index,'offset'] = offset[folder]
                    else:
                        idx.append(index)

                features.drop(index=idx, inplace=True)

                # save dataframe as pickle
                features.to_pickle(file + '_offset_per_cell.pkl')
            
            bar.next()


def generate_pickle(city, corPos=True, mappedHeights=True):
    # read json files and generate pickle file
    print("generate pickle files")

    if city == 'do':
        folder_ending = ''
        if not mappedHeights:
            folder_ending += '_noHeights'
        if not corPos:
            folder_ending += '_noPositionCorrection'
        
        path = args.base_path + 'dataset_engineered' + folder_ending + '/dortmund/'

        for area in args.areas:
            features = pd.DataFrame()
            folder = path + area + '/features/'
            bar = IncrementalBar('read features/' + area, max = len(os.listdir(folder)))

            for filename in os.listdir(folder):
                if filename.endswith('.json'):
                    with open(folder + filename) as f:
                        vals = json.load(f)
                    for val in vals:
                        vals[val] = [vals[val]]

                    vals['id'] = [int(filename.split('.')[0])]
                    vals['city'] = ['dortmund']
                    vals['area'] = [area]

                    if filename.split('.')[0][0:-4] == '12918809' and (vals['RSRP'][0] == -113 or vals['RSRP'][0] == -124):
                        continue

                    if features.empty:
                        features = pd.DataFrame.from_dict(vals, orient='columns')
                    else:
                        tmp = pd.DataFrame.from_dict(vals, orient='columns')
                        features = features.append(tmp, ignore_index=True)

                bar.next()

            # save dataframe as pickle
            features.to_pickle(args.base_path + 'dataset_processed' + folder_ending + '/dortmund/' + area + '.pkl')
            print('\n saved ' + area + ' as pkl')

    elif city == 'aa':
        path = args.base_path + 'dataset_engineered/aarhus/'

        for month in args.folders:
            for height in args.heights:
                features = pd.DataFrame()
                folder = path + month + '/' + height + '/features/'

                if not os.path.exists(folder):
                    continue

                bar = IncrementalBar('read features/' + month + ' - ' + height, max = len(os.listdir(folder)))

                for filename in os.listdir(folder):
                    if filename.endswith('.json'):
                        with open(folder + filename) as f:
                            vals = json.load(f)
                        for val in vals:
                            vals[val] = [vals[val]]

                        vals['id'] = [int(filename.split('.')[0])]
                        vals['city'] = ['aarhus']
                        vals['area'] = [month]

                        if features.empty:
                            features = pd.DataFrame.from_dict(vals, orient='columns')
                        else:
                            tmp = pd.DataFrame.from_dict(vals, orient='columns')
                            features = features.append(tmp, ignore_index=True)

                    bar.next()

                # save dataframe as pickle
                features.to_pickle(args.base_path + 'dataset_processed/aarhus/' + month + '_' + height + '.pkl')
                print('\n saved ' + month + '_' +  height + ' as pkl')

    elif city == 'ko':
        path = args.base_path + 'dataset_engineered/kopenhagen/'

        
        features = pd.DataFrame()
        folder = path + '/features/'
        bar = IncrementalBar('read features' , max = len(os.listdir(folder)))

        for filename in os.listdir(folder):
            if filename.endswith('.json'):
                with open(folder + filename) as f:
                    vals = json.load(f)
                for val in vals:
                    vals[val] = [vals[val]]

                vals['id'] = [int(filename.split('.')[0])]
                vals['city'] = ['kopenhagen']
                vals['area'] = ['']

                if features.empty:
                    features = pd.DataFrame.from_dict(vals, orient='columns')
                else:
                    tmp = pd.DataFrame.from_dict(vals, orient='columns')
                    features = features.append(tmp, ignore_index=True)

            bar.next()

        # save dataframe as pickle
        features.to_pickle(args.base_path + 'dataset_processed/kopenhagen/kopenhagen.pkl')
        print('\n saved kopenhagen as pkl')

    elif city == 'wu':
        path = args.base_path + 'dataset_engineered/wuppertal/'
        
        features = pd.DataFrame()
        folder = path + '/features/'
        bar = IncrementalBar('read features' , max = len(os.listdir(folder)))

        for filename in os.listdir(folder):
            if filename.endswith('.json'):
                with open(folder + filename) as f:
                    vals = json.load(f)
                for val in vals:
                    vals[val] = [vals[val]]

                vals['id'] = [int(filename.split('.')[0])]
                vals['city'] = ['wuppertal']
                vals['area'] = ['']

                if features.empty:
                    features = pd.DataFrame.from_dict(vals, orient='columns')
                else:
                    tmp = pd.DataFrame.from_dict(vals, orient='columns')
                    features = features.append(tmp, ignore_index=True)

            bar.next()

        # save dataframe as pickle
        features.to_pickle(args.base_path + 'dataset_processed/wuppertal/wuppertal.pkl')
        print('\n saved wuppertal as pkl')


def generate_csv(city, offset_per_cell=False, REM=False):
    file_ending = ''
    if offset_per_cell:
        file_ending += '_offset_per_cell'
    if REM:
        file_ending += '_REM'

    if city == 'do':
        for area in args.areas:
            path = args.base_path + '/dataset_processed/dortmund/' + area + file_ending + '.pkl'
            features = pd.read_pickle(path)
            tmp = []

            for file in os.listdir(args.base_path + 'dataset/dortmund/' + area):
                if 'cells' not in file:
                    with open(args.base_path + 'dataset/dortmund/' + area + '/' + file, 'r') as f:
                        reader = csv.reader(f)
                        ids = [int(x[0]) for x in list(reader)[1::]]

                    for id in ids:
                        tmp.append(features.loc[features['id'] == id].values.tolist()[0])

            # save dataframe as pickle
            with open(args.base_path + 'dataset_processed/' + area + file_ending + '.csv', 'w+', newline='') as f:
                wr = csv.writer(f)
                wr.writerow(features.columns.to_list())

                for elem in tmp:
                    wr.writerow(elem)

            print('\n saved ' + area + ' as csv')


def process_data_REM(path, args):
    print('read data')

    samples = pd.read_pickle(path)
    print(str(len(samples)) + ' samples')

    # add / change columns
    samples = edit_columns(args, samples)

    # define used columns
    columns = ["bandwidth", "frequency", "dist3d", "delta_lon", "delta_lat", "delta_elevation", "cell_height", "height"]#
    if args.engineered_features:
        columns += ["indoorDist", "indoorIntersections", "terrainDist", "terrainIntersections"]
    if args.pathloss_model:
        columns += ["pathloss"]
    # other columns: ref, area, city, id, offset

    print('columns used:')
    print(columns)
    
    return samples


def process_data(args, seed=200):
    print('read data')

    file_ending_do = ''
    if args.offset_per_cell:
        file_ending_do += '_offset_per_cell'
    if args.REM:
        file_ending_do += '_REM_' + str(args.REM_size)

    file_ending_do += '.pkl'

    file_ending = ''
    if args.offset_per_cell:
        file_ending += '_offset_per_cell'

    file_ending += '.pkl'

    folder_ending = ''
    try:
        if not args.mapped_heights:
            folder_ending += '_noHeights'
        if not args.corrected_position:
            folder_ending += '_noPositionCorrection'
    except AttributeError:
        folder_ending = ''
    
    path_dortmund = args.base_path + 'dataset_processed' + folder_ending + '/dortmund/'
    path_aarhus = args.base_path + 'dataset_processed/aarhus/'
    path_kopenhagen = args.base_path + 'dataset_processed/kopenhagen/'
    path_wuppertal = args.base_path + 'dataset_processed/wuppertal/'

    samples = pd.DataFrame()
    val_samples = pd.DataFrame()
    test_samples = pd.DataFrame()

    # read dortmund data
    if args.dortmund:
        for area in args.areas:
            if args.testset is not None and (area == args.testset or args.testset == 'dortmund'):
                if test_samples.empty:
                    test_samples = pd.read_pickle(path_dortmund + area + file_ending_do)
                else:
                    tmp = pd.read_pickle(path_dortmund + area + file_ending_do)
                    test_samples = test_samples.append(tmp, ignore_index=True)
            elif args.valset is not None and (area == args.valset or args.valset == 'dortmund'):
                if val_samples.empty:
                    val_samples = pd.read_pickle(path_dortmund + area + file_ending_do)
                else:
                    tmp = pd.read_pickle(path_dortmund + area + file_ending_do)
                    val_samples = val_samples.append(tmp, ignore_index=True)
            else:
                if samples.empty:
                    samples = pd.read_pickle(path_dortmund + area + file_ending_do)
                else:
                    tmp = pd.read_pickle(path_dortmund + area + file_ending_do)
                    samples = samples.append(tmp, ignore_index=True)

    # read aarhus data
    if args.aarhus:
        for month in args.folders:
            for height in args.heights:
                path = path_aarhus + month + '_' + height + file_ending
                if os.path.exists(path):
                    if args.testset is not None and (month == args.testset or height == args.testset or month + '_' + height == args.testset or 'aarhus' == args.testset):
                        if test_samples.empty:
                            test_samples = pd.read_pickle(path)
                        else:
                            tmp = pd.read_pickle(path)
                            test_samples = test_samples.append(tmp, ignore_index=True)
                    elif args.valset is not None and (month == args.valset or height == args.valset or month + '_' + height == args.valset or 'aarhus' == args.valset):
                        if val_samples.empty:
                            val_samples = pd.read_pickle(path)
                        else:
                            tmp = pd.read_pickle(path)
                            val_samples = val_samples.append(tmp, ignore_index=True)
                    else:
                        if samples.empty:
                            samples = pd.read_pickle(path)
                        else:
                            tmp = pd.read_pickle(path)
                            samples = samples.append(tmp, ignore_index=True)

    # read kopenhagen data
    if args.kopenhagen:
        if args.testset is not None and args.testset == 'kopenhagen':
            test_samples = pd.read_pickle(path_kopenhagen + 'kopenhagen'+ file_ending)
        elif args.valset is not None and args.valset == 'kopenhagen':
            val_samples = pd.read_pickle(path_kopenhagen + 'kopenhagen' + file_ending)
        else:
            if samples.empty:
                samples = pd.read_pickle(path_kopenhagen + 'kopenhagen' + file_ending)
            else:
                tmp = pd.read_pickle(path_kopenhagen + 'kopenhagen' + file_ending)
                samples = samples.append(tmp, ignore_index=True)
                
    # read wuppertal data
    try:
        if args.wuppertal:
            if args.testset is not None and args.testset == 'wuppertal':
                test_samples = pd.read_pickle(path_wuppertal + 'wuppertal'+ file_ending_do)
            elif args.valset is not None and args.valset == 'wuppertal':
                val_samples = pd.read_pickle(path_wuppertal + 'wuppertal' + file_ending_do)
            else:
                if samples.empty:
                    samples = pd.read_pickle(path_wuppertal + 'wuppertal' + file_ending_do)
                else:
                    tmp = pd.read_pickle(path_wuppertal + 'wuppertal' + file_ending_do)
                    samples = samples.append(tmp, ignore_index=True)
    except AttributeError:
        pass

    # check if test set / size is defined
    if (args.test_size is None and args.testset is None) or (args.val_size is None and args.valset is None) or (args.valset == args.testset and args.valset is not None):
        # return whole data as train set
        return edit_columns(samples)

    if (args.valset is not None and len(val_samples) == 0) or (args.testset is not None and len(test_samples) == 0):
        raise Exception('invalid validation or test set name')

    # split data
    num_samples = len(samples) + len(test_samples) + len(val_samples)
    print(str(num_samples) + ' samples')

    try:
        if args.drop_data > 0:
            samples, _ = train_test_split(samples, test_size=args.drop_data, shuffle=True, random_state=seed)
            num_samples = len(samples) + len(test_samples) + len(val_samples)
            print(str(num_samples) + ' samples after dropping')
    except AttributeError:
        pass


    if args.valset is not None and args.testset is None:
        train_set, test_set = train_test_split(samples, test_size=args.test_size, shuffle=True, random_state=seed)
        val_set = val_samples
    elif args.valset is None and args.testset is not None:
        train_set, val_set = train_test_split(samples, test_size=args.val_size, shuffle=True, random_state=seed)
        test_set = test_samples
    elif args.valset is None and args.testset is None:
        if args.test_size > 0:
            train_set, test_set = train_test_split(samples, test_size=args.test_size, shuffle=True, random_state=seed)
            train_set, val_set = train_test_split(train_set, test_size=args.val_size*(1/(1-args.test_size)), shuffle=True, random_state=seed)
        else:
            test_set = samples.copy().loc[0:2]
            train_set, val_set = train_test_split(samples, test_size=args.val_size, shuffle=True, random_state=seed)
    else:
        train_set = samples
        test_set = test_samples
        val_set = val_samples
    
    # add / change columns
    train_set = edit_columns(args, train_set)
    test_set = edit_columns(args, test_set)
    val_set = edit_columns(args, val_set)

    print('length train set: ' + str(len(train_set)))
    print('length val set: ' + str(len(val_set)))
    print('length test set: ' + str(len(test_set)))

    # define used columns
    columns = ["bandwidth", "frequency", "dist3d", "delta_lon", "delta_lat", "delta_elevation", "cell_height", "height"]#
    if args.engineered_features:
        columns += ["indoorDist", "indoorIntersections", "terrainDist", "terrainIntersections"]
    if args.pathloss_model:
        columns += ["pathloss"]
    # other columns: ref, area, city, id, offset

    print('columns used:')
    print(columns)
    
    # define scalers based on train data
    scaled_features = train_set[columns]
    scaled_targets = train_set[["y"]]
    input_scaler = StandardScaler().fit(scaled_features)
    target_scaler = StandardScaler().fit(scaled_targets)
                
    return train_set, val_set, test_set, input_scaler, target_scaler, columns



def edit_columns(args, samples):
    # add path loss
    B = samples["bandwidth"].to_numpy()
    f = samples["frequency"].to_numpy()
    offset = samples["offset"].to_numpy()
    dist = samples["dist3d"].to_numpy()
    lat_dist = samples["lat_dist"].to_numpy()
    lon_dist = samples["lon_dist"].to_numpy()
    UE_height = samples["height"].to_numpy()

    dist_2d = np.sqrt(np.power(lat_dist, 2) + np.power(lon_dist, 2))
    Ps = np.ones(B.shape) * 40
    L = np.zeros(Ps.shape)
    ref = np.zeros(Ps.shape)

    for i in range(len(Ps)):       
        L[i] = UMa_B_NLOS_pathloss([dist_2d[i]], [dist[i]], f[i] * 1e6, h_UT = UE_height[i]) - offset[i]
        ref[i] = Ps[i] - get_rsrp_ref(B[i])

    samples["pathloss"] = pd.Series(ref-L, index=samples.index)
    samples["ref"] = pd.Series(ref, index=samples.index)
    samples["dist2d"] = pd.Series(dist_2d, index=samples.index)

    # switch to delta values
    samples["delta_lat"] = pd.Series(np.abs(samples["cell_lat"].to_numpy() - samples["lat"].to_numpy()), index=samples.index)
    samples["delta_lon"] = pd.Series(np.abs(samples["cell_lon"].to_numpy() - samples["lon"].to_numpy()), index=samples.index)
    samples["delta_elevation"] = pd.Series((samples["cell_altitude"].to_numpy() - samples["cell_height"].to_numpy()) - (samples["altitude"].to_numpy() - samples["height"].to_numpy()), index=samples.index)
    
    # drop columns
    samples.drop(columns=["lat_dist", "lon_dist", "lat", "cell_lat", "lon", "cell_lon", "cell_altitude", "altitude"], inplace=True)

    # define target value
    if args.pathloss_model:
        samples["y"] = pd.Series(samples["RSRP"].to_numpy() - samples["pathloss"].to_numpy(), index=samples.index)
    else:
        samples["y"] = samples["RSRP"]

    return samples



def process_images_REM(path, size, _range, enable_threading=True, number_threads=8, sv=True):
    # identify folder
    if _range != -1:
        folder = str(_range) + 'm/'
    else:
        folder = 'full/'

    if sv:
        folder = '/sv/' + folder
    else:
        folder = '/tv/' + folder

    for dir in os.listdir(path):
        print(dir)

        if os.path.isdir(path + dir):
            _path = path + dir + '/engineered' + folder

            if enable_threading:
                # threading enabled: start multithreading
                task_queue = TaskQueue(number_threads)

                for filename in os.listdir(path):
                    task_queue.add_task((args, 'REM', _path, size, folder))

                task_queue.start_workers()
                task_queue.join()

            else:
                # no multithreading enabled: iterate through eps files
                bar = IncrementalBar('process images kopenhagen', max = len(os.listdir(_path)))

                for filename in os.listdir(path):
                    if filename.endswith('.eps'):
                        eps2png(args, 'REM', size, folder, _path + filename)
                    bar.next()


def process_images(city, size, _range, enable_threading=True, number_threads=8, sv=True, corPos=True, mappedHeights=True):
    # identify folder
    if _range != -1:
        folder = str(_range) + 'm/'
    else:
        folder = 'full/'

    if sv:
        folder = '/sv/' + folder
    else:
        folder = '/tv/' + folder

    if city == 'do':
        folder_ending = ''
        if not mappedHeights:
            folder_ending += '_noHeights'
        if not corPos:
            folder_ending += '_noPositionCorrection'

        path_dortmund = args.base_path + 'dataset_engineered' + folder_ending + '/dortmund/'

        if enable_threading:
            # threading enabled: start multithreading
            for area in args.areas:
                print(area + ' started')
                path = path_dortmund + area + folder

                task_queue = TaskQueue(number_threads)

                for filename in os.listdir(path):
                    task_queue.add_task((args, city, path + filename, size, folder, folder_ending))

                task_queue.start_workers()
                task_queue.join()

        else:
            # no multithreading enabled: iterate through eps files
            for area in args.areas:
                path = path_dortmund + area + folder
                bar = IncrementalBar('process images dortmund/' + area, max = len(os.listdir(path)))

                for filename in os.listdir(path):
                    if filename.endswith('.eps'):
                        eps2png(args, city, size, folder, path + filename, folder_ending)
                    bar.next()

    elif city == 'aa':
        path_aarhus = args.base_path + 'dataset_engineered/aarhus/'

        if enable_threading:
            # threading enabled: start multithreading
            for month in args.folders:
                for height in args.heights:
                    print(month + '-' + height + ' started')

                    path = path_aarhus + month + '/' + height + folder
                    if not os.path.exists(path):
                        continue

                    task_queue = TaskQueue(number_threads)

                    for filename in os.listdir(path):
                        task_queue.add_task((args, city, path + filename, size, folder))

                    task_queue.start_workers()
                    task_queue.join()

        else:
            # no multithreading enabled: iterate through eps files
            for month in args.folders:
                for height in args.heights:
                    path = path_aarhus + month + '/' + height + folder
                    bar = IncrementalBar('process images aarhus/' + month + ', ' + height, max = len(os.listdir(path)))

                    for filename in os.listdir(path):
                        if filename.endswith('.eps'):
                            eps2png(args, city, size, folder, path + filename)
                        bar.next()

    elif city == 'ko':
        path_kopenhagen = args.base_path + 'dataset_engineered/kopenhagen/'

        if enable_threading:
            # threading enabled: start multithreading
            path = path_kopenhagen + folder
            task_queue = TaskQueue(number_threads)

            for filename in os.listdir(path):
                task_queue.add_task((args, city, path + filename, size, folder))

            task_queue.start_workers()
            task_queue.join()

        else:
            # no multithreading enabled: iterate through eps files
            path = path_kopenhagen + folder
            bar = IncrementalBar('process images kopenhagen', max = len(os.listdir(path)))

            for filename in os.listdir(path):
                if filename.endswith('.eps'):
                    eps2png(args, city, size, folder, path + filename)
                bar.next()

    elif city == 'wu':
        path_wuppertal = args.base_path + 'dataset_engineered/wuppertal/'

        if enable_threading:
            # threading enabled: start multithreading
            path = path_wuppertal + folder
            task_queue = TaskQueue(number_threads)

            for filename in os.listdir(path):
                task_queue.add_task((args, city, path + filename, size, folder))

            task_queue.start_workers()
            task_queue.join()

        else:
            # no multithreading enabled: iterate through eps files
            path = path_wuppertal + folder
            bar = IncrementalBar('process images wuppertal', max = len(os.listdir(path)))

            for filename in os.listdir(path):
                if filename.endswith('.eps'):
                    eps2png(args, city, size, folder, path + filename)
                bar.next()




if __name__ == '__main__':
    args = init_model.init_args()

    generate_pickle(args.city)
    # adapt_pickle_offset(args.city)
    # adapt_pickle_REM(args.city, args.REM_size, True)

    for _range in args.ranges:
        print(_range)
        process_images(args.city, 64, _range, sv=False, enable_threading=True, number_threads=10)
        process_images(args.city, 64, _range, sv=True, enable_threading=True, number_threads=10)