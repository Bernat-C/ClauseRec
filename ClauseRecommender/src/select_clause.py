import os
import argparse
import torch
from torch_geometric.nn import to_hetero

from clauserec import ClauseRec
from dataset import read_test_instance

def getClausePredictions(instance, model_file):
    
    data = instance.getHeteroData()
    
    model = ClauseRec(hidden_channels=64)
    model.load_state_dict(torch.load(model_file,map_location=torch.device('cpu')),strict=False)
    model = to_hetero(model, data.metadata(),aggr='sum')
    model.eval()

    out = model(data.x_dict, data.edge_index_dict)
    return out['clauses']

def getBestConflict(instance, predictions, conflicts): 
    
    index = -1
    max_qual = -1

    for i, conflict in enumerate(conflicts):
        try:
            curr = instance.clauses.index(frozenset(conflict))
            if predictions[curr][0] > max_qual:
                index = i
                max_qual = predictions[curr][0]

        except ValueError:
            pass

    return index, max_qual

def getClause(dimacs, features, model_file, conflicts: list[frozenset[str]]):

    instance = read_test_instance(dimacs, features, conflicts)
    
    data = instance.getHeteroData()
    
    #model = torch.load(os.path.join(dataset.root,"models","20240410-091906.pt"))
    model = ClauseRec(hidden_channels=64)

    model.load_state_dict(torch.load(model_file,map_location=torch.device('cpu')),strict=False)
    
    model = to_hetero(model, data.metadata(),aggr='sum')
    model.eval()
    # model = torch.load(path_model)
    # model = to_hetero(model, data.metadata(),aggr='sum')
    # model.eval()

    out = model(data.x_dict, data.edge_index_dict)
        
    index = -1
    max_qual = -1

    for i, conflict in enumerate(conflicts):
        try:
            curr = instance.clauses.index(frozenset(conflict))
            if out['clauses'][curr][0] > max_qual:
                index = i
                max_qual = out['clauses'][curr][0]

        except ValueError:
            pass

    return index, max_qual

def call(conflicts, instance_name: str):

    dimacs = os.path.join(os.path.dirname(__file__),'../..',f'solversjordi/temp/{instance_name}.dimacs')
    features = os.path.join(os.path.dirname(__file__),'../..',f'solversjordi/temp/{instance_name}.features')
    model = os.path.join(os.path.dirname(__file__),"..","data","models","Regressor.pt")#"20240425-154025.pt")
    
    instance = read_test_instance(dimacs, features, conflicts)
    conflicts = [[x for x in sublist if x != 0] for sublist in conflicts]
    clause_predictions = getClausePredictions(instance, model)
    index, max_qual = getBestConflict(instance, clause_predictions, conflicts)
    
    return index

if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("dimacs", nargs='?', default=os.path.join(os.path.dirname(__file__),'../../solved/j30rcpbrief/maple.3600.ub/J301_8.RCP.dimacs'))
    parser.add_argument("features", nargs='?', default=os.path.join(os.path.dirname(__file__),'../../solved/j30rcpbrief/maple.3600.ub/J301_8.RCP.features'))
    parser.add_argument("model", nargs='?', default=os.path.join(os.path.dirname(__file__),"../data","models","20240425-154025.pt"))

    args=parser.parse_args()
    
    conflicts = []
    conflicts.append(frozenset(('-1','2')))
    conflicts.append(frozenset(('5','-40')))
    conflicts.append(frozenset(('158', '160' '-164')))
    
    i, max_qual = getClause(args.dimacs, args.features, args.model, conflicts)

    if i != -1:
        print(f"The selected conflict is {conflicts[i]} with a value of {max_qual}.")
    else:
        print("The selected conflict does not appear in the list.")
