from types import SimpleNamespace


def init_args():
    args = SimpleNamespace()

    args.base_path = '../data/'

    # process data
    args.city = 'do'
    args.image_sizes = [64]
    args.ranges = [300]
    args.folders =  ["August","October"] # aarhus
    args.heights = ["10m","15m", "20m", "25m", "30m", "35m", "40m", "100m", "DT"] # aarhus
    args.areas = ["campus", "highway", "urban", "suburban"] # dortmund

    # evaluate
    args.exps = "global"
    args.final_file = "test.csv"

    # REM
    args.REM_folder = '/Dortmund/urban/tmobile/from_list/'

    # --------- ML model parameters -----------------
    args.cuda = False
    args.wandb = False
    args.wandb_project = "ml"
    args.kernels = [5,3,3,3,3,2]
    args.max_pool = [2,2,2,2,2,2]
    args.out_channels = [32,16,16,16,10,1]
    args.nn_layers = [256,128,64,32]

    # model parameters
    args.pathloss_model = True
    args.engineered_features = True
    args.im_model = True
    args.feature_model = True

    # data modes
    args.mapped_heights = True
    args.corrected_position = True
    args.offset_per_cell = True
    args.REM = True
    args.REM_size = 5

    # cnn parameters
    args.dilation = 1
    args.padding = 3
    args.stride = 1
    args.data_augmentation = 0

    # data parameters
    args.dortmund = True
    args.aarhus = False
    args.kopenhagen = False
    args.wuppertal = False
    args.seed = 35
    args.batch_size = 128

    # image parameters
    args.sv = True
    args.tv = True
    args.image_size = 64
    args.image_range = [300, 300]

    # segmentation parameters 
    args.test_size = 0.1
    args.testset = None
    args.val_size = 0.1
    args.valset = None
    args.test = False
    args.drop_data = 0

    # train parameters
    args.num_workers = 6
    args.epochs = 300
    args.weight_decay = 5e-4
    args.learning_rate = 1e-3
    args.dropout = 0

    # dataloader
    args.load_images = False
    args.use_multi_epochs_loader = True

    return args