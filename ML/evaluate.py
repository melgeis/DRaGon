import os
import types
import process_data

import numpy as np
import pandas as pd

import torch
from torch import nn
from torch import optim

from sklearn.utils import shuffle
from sklearn.metrics import mean_squared_error, mean_absolute_error
from tqdm import tqdm
from experimentlogger import load_experiment
from easydict import EasyDict as edict

import train
import model_dragon
import init_model
from helpers_ML import *
from dataset import MyDataset




def evaluate(args, test_loader, model, target_scaler, input_scaler):
    bar = tqdm(total = len(test_loader))
    RSRP = dict()
    RSRP["measured"] = []
    RSRP["DNN"] = []
    RSRP["id"] = []
    RSRP["area"] = []
    RSRP["frequency"] = []
    RSRP["error"] = []
    RSRP["height"] = []
    RSRP["city"] = []


    with torch.no_grad():
        for idx, (features, image, target, _RSRP, PL, ref, offset, dist2d, id, f, area, height, city) in enumerate(test_loader):
            if args.cuda:
                features = features.cuda()
                image = image.cuda()

            output = model(image, features)

            if args.pathloss_model:
                RSRP["DNN"] += (torch.tensor([elem[0] for elem in target_scaler.inverse_transform(output.cpu().numpy())]) + PL).tolist()
            else:
                RSRP["DNN"] += (torch.tensor([elem[0] for elem in target_scaler.inverse_transform(output.cpu().numpy())])).tolist()

            RSRP["measured"] +=  _RSRP.tolist()
            RSRP["id"] += id.tolist()

            if args.pathloss_model:
                RSRP["error"] += (torch.tensor([elem[0] for elem in target_scaler.inverse_transform(output.cpu().numpy())]) + PL - _RSRP).tolist()
            else:
                RSRP["error"] += (torch.tensor([elem[0] for elem in target_scaler.inverse_transform(output.cpu().numpy())]) - _RSRP).tolist()

            RSRP["frequency"] += f.tolist()
            RSRP["area"] += [x for x in area]
            RSRP["height"] += height.tolist()
            RSRP["city"] += [x for x in city]

            try:
                outputs = torch.cat([outputs, output],0)
            except:
                outputs = output

            bar.update(1)
    
    bar.close()

    return outputs, RSRP



def run(_args):
    if _args.cuda and torch.cuda.is_available():
        cuda = True
        print("CUDA enabled")
    else:
        cuda = False
        print("CUDA disabled")

    # Load experiment
    exp = load_experiment(_args.exps, root_path=_args.base_path + 'models/results/exps/')
    name = _args.exps
    final_file = _args.final_file
    pre = ''#_args.filename
    args = edict(exp.config)
    args.name = name.split('_')[0]
    args.cuda = cuda
    args.base_path = _args.base_path

    try: 
        args.areas
    except AttributeError:
        args.areas = _args.areas

    try: 
        args.folders
    except AttributeError:
        args.folders = _args.folders

    try: 
        args.heights
    except AttributeError:
        args.heights = _args.heights


    # get Data
    _, _, test_set, input_scaler, target_scaler, columns = process_data.process_data(args, seed=args.seed)
    test_dataset = MyDataset(args.base_path, test_set, input_scaler, target_scaler, args.image_size, args.image_range, columns, args.im_model, 0, args.load_images, args.sv, args.tv, args.num_workers, True)

    # load data
    test_loader = torch.utils.data.DataLoader(test_dataset, batch_size=args.batch_size, shuffle=False, num_workers=args.num_workers, drop_last=False)

    # init model
    model = model_dragon.CombiModel(args, target_scaler)

    if args.cuda:
        model.cuda()

    # find model name
    list_of_files = os.listdir(args.base_path + 'models/results/exps/models/')

    if '/' in args.name:
        args.name = args.name.split('/')[1]

    for filename in list_of_files:
        if filename.startswith(args.name):  
            model_name = filename
            print(model_name)
            break
    
    # load model state
    if args.cuda:
        model.load_state_dict(torch.load(args.base_path + 'models/results/exps/models/' + model_name))
    else:
        model.load_state_dict(torch.load(args.base_path + 'models/results/exps/models/' + model_name, map_location=torch.device('cpu')))
    model.eval()

    # evaluate test data
    output, RSRP = evaluate(args, test_loader, model, target_scaler, input_scaler)
    
    # save results
    RSRP_df = pd.DataFrame.from_dict(data=RSRP, orient='columns')
    RSRP_df.to_csv(args.base_path + 'models/results/exps/evaluated/' + _args.final_file)   


if __name__ == '__main__':
    args = init_model.init_args()
    run(args)