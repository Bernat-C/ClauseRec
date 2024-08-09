import os
import random
import numpy as np
import torch
import pickle
from tqdm import tqdm

from torch_geometric.data import HeteroData, Dataset
import torch_geometric.transforms as T

class DimacsDataset(Dataset):
    
    def __init__(self, root, folder, nmax=1000, transform=None, pre_transform=None):
        """
        root = Where the dataset should be stored. This folder is split
        into raw_dir (downloaded dataset) and processed_dir (processed data). 
        """
        self.root = root
        self.length = 0
        self.nmax = nmax
        self.folder = folder
        super(DimacsDataset, self).__init__(root, transform, pre_transform)
        
    @property
    def raw_file_names(self):
        """ If this file exists in raw_dir, the download is not triggered."""
        files = [f'data_{i}.pt' for i in range(self.nmax)]
        self.length = len(files)
        return files
    @property
    def processed_file_names(self):
        """ If these files are found in raw_dir, processing is skipped"""
        return "data"

    def getMetadata(self):
        graph = torch.load(os.path.join(self.raw_dir, f'data_0.pt'))
        return graph.metadata()
        
    def download(self):
        instances = load_folder(self.folder,self.nmax)
        self.length = len(instances)
        
        for index, instance in enumerate(instances):
            
            data = instance.getHeteroData()
            torch.save(data, os.path.join(self.raw_dir, f'data_{instance.name}.pt'))

    def process(self):
        pass

    def len(self):
        return self.length

    def get(self, idx):
        """ - Equivalent to __getitem__ in pytorch
        """
        graph = torch.load(os.path.join(self.raw_dir, f'data_{idx}.pt'))
        return graph

class Instance:
    def __init__(self):
        self.name = ""
        self.num_clauses = 0
        self.num_lits = 0
        
        self.clauses = []
        self.x = []
        self.clause_edge_index = []
        self.not_var_edge_index = []
        self.y = []
        
        # Emmagatzema quin id de literal té cada literal que afegim.
        self.dict_lit_var = {}
        
    def setName(self,name):
        self.name = name
    
    def addVariable(self,id,features):
        self.x.append(np.array(features))
        self.x.append(np.array(features))
        
        self.dict_lit_var[int(id)] = self.num_lits
        self.dict_lit_var[-int(id)] = self.num_lits+1
        self.addEdgeNegLiterals(self.num_lits,self.num_lits+1)
        
        self.num_lits += 2
            
    def addEdgeNegLiterals(self,pos_id,neg_id):
        self.not_var_edge_index.append(np.array([pos_id,neg_id],dtype=int))
    
    def addEdgeVarClause(self,id_clause,id_var):
        self.clause_edge_index.append(np.array([id_clause,id_var],dtype=int))
            
    def addClause(self,literals,label):
        self.clauses.append(frozenset(literals))
        self.num_clauses += 1 
        id_clause = self.num_clauses -1
        for lit in literals:
            self.addEdgeVarClause(id_clause,self.dict_lit_var[int(lit)])
        self.y.append(float(label))
        """ if round(float(label)) > 0:
            self.y.append(1)
        else:
            self.y.append(0) """
            
    def getHeteroData(self):
        # Create data object
        data = HeteroData()

        data['variables'].x = self.x.view(-1, 1).float()
        data['clauses'].x = torch.ones(self.num_clauses,dtype=torch.float32).view(-1, 1)#torch.empty(self.num_clauses,dtype=torch.float32).view(-1, 1)#

        data['clauses','have','variables'].edge_index = self.clause_edge_index.transpose(1, 0)
        data['variables','negated','variables'].edge_index = self.not_var_edge_index.transpose(1, 0)

        data['clauses'].y = self.y.float().view(-1,1)
        
        return T.ToUndirected()(data)
            
    def toTensors(self):
        self.x = torch.from_numpy(np.array(self.x, dtype=float))
        self.clause_edge_index = torch.from_numpy(np.array(self.clause_edge_index))
        self.not_var_edge_index = torch.from_numpy(np.array(self.not_var_edge_index))
        self.y = torch.from_numpy(np.array(self.y, dtype=float))
        
def read_test_instance(path_train, path_features, conflicts):
    instance = Instance()
    pickle_name = os.path.abspath(os.path.join("ClauseRecommender", "data","raw",os.path.basename(path_train + ".pkl")))

    if  os.path.exists(pickle_name):
        with open(pickle_name, 'rb') as f:
            return pickle.load(f)
        
    with open(path_features, 'r') as file:
        for line in file:
            line_data = line.split() # var feature1 ... featuren
            instance.addVariable(line_data[0],line_data[1:])

    with open(path_train, 'r') as file:
        next(file) # Skip first line of dimacs file
        for line in file:
            line_data = line.split() # lit1 ... litn 0 quality
            instance.addClause(line_data[0:-1],0)
            
        for c in conflicts:
            if not frozenset(c) in instance.clauses:
                instance.addClause(list(c),0)
        instance.toTensors()
        instance.setName(os.path.basename(path_train))

    print(f"----------------------------{pickle_name}")
    with open(pickle_name, 'wb') as f:
        pickle.dump(instance, f)
    return instance

def read_train_instance(path_train, path_features):
    instance = Instance()
    
    with open(path_features, 'r') as file:
        for line in file:
            line_data = line.split() # var feature1 ... featuren
            instance.addVariable(line_data[0],line_data[1:])

    with open(path_train, 'r') as file:
        for line in file:
            line_data = line.split() # lit1 ... litn 0 quality
            instance.addClause(line_data[0:-2],line_data[-1])
        instance.toTensors()
    return instance

def load_folder(folder,nmax):
    MAX_SIZE = 5000000
    dataset = []
    files = [f for f in os.listdir(folder) if f.endswith(".RCP.train") and MAX_SIZE > os.path.getsize(os.path.join(folder,f))]
    for filename in (pbar := tqdm(files)):
        if len(dataset) < nmax:
            path_features = os.path.join(folder,"".join([filename[:-6], ".features"]))
            pbar.set_description(f"Processing {filename}.")
            if os.path.exists(path_features):
                path_train = os.path.join(folder, filename)
                dataset.append(read_train_instance(path_train, path_features))
                pbar.set_description(f"The train file {filename} has been created.")
            else:
                pbar.set_description(f"The feature file {filename} was not found.")
    return np.array(dataset)

if __name__ == "__main__":

    directory = '../solved/j30rcpbrief/maple.3600.ub/'

    content = load_folder(directory)
    # Check the first graph
    data = content[0]  

    # Printing Stats
    print(f"\nliterals: {data.num_lits}")
    print(f"\nClauses: {data.num_clauses}")
    
    print("\nClause Edges:")
    print(data.clause_edge_index.shape)
    print(data.clause_edge_index)

    print("\nVar Edges:")
    print(data.not_var_edge_index.shape)
    print(data.not_var_edge_index)

    print("\nLabels:")
    print(data.y)

    print("\nFeatures:")
    print(data.x.shape)
    print(data.x)
    
    print(f'\nNumber of graphs: {len(content)}')
    print(f'Number of features: {data.x.shape[1]}')
    print(f'Number of classes: 2')
    print(50*'=')

    print(f'Number of nodes: {len(data.x)}')
    print(f'Number of edges: {len(data.clause_edge_index) + len(data.not_var_edge_index)}')
    
    dataset = DimacsDataset("../ClauseRecommender/data/",directory)

    dataset.data.validate()
    print(dataset.data)
    print(dataset.data.node_stores)
    print(dataset.data.edge_stores)
    print(dataset.data.metadata())
        
    """ split_index = int(len(dataset) * 0.8)
    
    train_set = dataset[:split_index]
    valid_set = dataset[split_index+1:]

    train_filename = os.path.join(args.data_dir,"train","t1.train")
    valid_filename = os.path.join(args.data_dir,"val","v1.train")
    
    store_as_pickle(train_set, train_filename)
    store_as_pickle(valid_set, valid_filename) """
