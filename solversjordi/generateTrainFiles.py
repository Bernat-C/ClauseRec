import os

"""
    The specified folder should exist and contain the ".dimacs" SAT file and the ".tc" tracecheck file.
    Processes all files generated with generate_instances.sh and writes the contents in a '.train' format used to train the NN.
    Every line of the '.train' file will be structured in the following form:
    clause 0 feature1 ... featuren n_tree
    Where all clauses appearing in the dimacs file as well as the tracecheck will appear without repetitions.
    n_tree will be the number of times the clause appears in the solving tree. 
    
    python3 generateTrainFiles.py ../solved/j30rcpbrief/maple.3600.ub
"""

# Elements will contain the clauses in the tracecheck file indexed by the number it assigns them
# occurrences will contain the number of occurrences of each clause in the tracecheck file indexed by the number assigned
def parse_line(line, elements: dict, occurrences: dict):
    numbers = list(map(int, line.split()))
    first_zero_index = numbers.index(0)
    second_zero_index = len(numbers) - 1 - numbers[::-1].index(0)
    
    if len(numbers[1:first_zero_index]) != 0:
        if not numbers[0] in elements:
            elements[numbers[0]] = frozenset(numbers[1:first_zero_index])
        
    for n in numbers[first_zero_index+1:second_zero_index]:
        if n in occurrences:
            occurrences[n] += 1
        else:
            occurrences[n] = 1
            
    return frozenset(numbers[1:first_zero_index])
            
def write_file(filename, apps: dict):
    with open(filename, 'w') as file:
        for c, app in apps.items():
            for l in c:
                file.write(str(l) + ' ')
            file.write('0')
            file.write(' ' + str(app))
            file.write('\n')
            
    print(f"{filename} was created.")

# Function to process files in the given folder path
def process_files_in_folder(folder_path):
    for filename in os.listdir(folder_path):
        if filename.endswith('.tc'):
            print(f"Try: {filename}")
            file_trace = os.path.join(folder_path, filename)
            file_dimacs = os.path.join(folder_path, os.path.splitext(filename)[0]+'.dimacs')
            file_train = os.path.join(folder_path, os.path.splitext(filename)[0]+'.train')
            if not os.path.isfile(file_train) and os.path.isfile(file_trace) and os.path.isfile(file_dimacs) and os.path.getsize(file_trace) != 0 and os.path.getsize(file_trace) < 1000000000:
                apps = {}
                
                # Parse Dimacs file
                with open(file_dimacs, 'r') as file:
                    next(file)
                    for line in file:
                        line_content = line.split(" 0")
                        ls = frozenset(list(map(int, line_content[0].split())))
                        
                        # Storing all features
                        if not ls in apps.keys():
                            apps[ls] = 0
                            
                # Parse tracecheck file
                max_value = 1 # Maximum number of appearances of a clause, used to normalize
                with open(file_trace, 'r') as file:
                    elements = {} # 23 -> clause(1,2,3)
                    occurrences = {} # 23 -> 3 times    
                    for line in file:
                        clause = parse_line(line, elements, occurrences)
                        if not clause in apps.keys():
                            apps[clause] = 0
                    
                    # Iterate over the found appearences
                    for n, app in occurrences.items():
                        apps[elements[n]] += app
                        if apps[elements[n]] > max_value:
                            max_value = apps[elements[n]]

                file_output = os.path.join(folder_path, os.path.splitext(filename)[0]+'.train')
                
                # NORMALIZATION
                for feature in apps.values():
                    feature /= max_value

                write_file(file_output, apps)
                    

if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        process_files_in_folder(sys.argv[1])
    else:
        print("Please provide the folder path as a parameter.")
