import random

def precision_at_k(recommended, relevant, k):

    recommended_at_k = recommended[:k]

    num_relevant_at_k = len(set(recommended_at_k).intersection(relevant))

    return num_relevant_at_k / k if k > 0 else 0

def recall_at_k(recommended, relevant, k, total_relevant):

    recommended_at_k = recommended[:k]

    num_relevant_at_k = len(set(recommended_at_k).intersection(relevant))

    return num_relevant_at_k / total_relevant if total_relevant > 0 else 0

def validate(batch, model, num_iterations=500, num_confs=10, K=3): 
    
    precisionK = recallK = total = 0    
    out = model(batch.x_dict, batch.edge_index_dict)
    
    for j in range(num_iterations): # Do it many times for every instances because the conflicts are random.    
        conflicts = [random.randint(0, batch['clauses'].y.shape[0]-1) for _ in range(num_confs)]
            
        conflicts_pred = []
        quals_pred = []
        quals_label = []

        for conflict in conflicts:
            conflicts_pred.append(conflict)
            quals_pred.append(out['clauses'][conflict][0])
            quals_label.append(batch['clauses']['y'][conflict])
        
        predictions_sorted = [x for _, x in sorted(zip(quals_pred, conflicts_pred))]
        labels_sorted = [x for _, x in sorted(zip(quals_label, conflicts_pred))]
        precisionK += precision_at_k(predictions_sorted,labels_sorted[:K],K)
        recallK += recall_at_k(predictions_sorted,labels_sorted[:K],K,sum(map(lambda x : x>0.7, labels_sorted)))
        total += 1
    
    return precisionK/total, recallK/total