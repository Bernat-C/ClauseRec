#include "mmkp.h"
#include <algorithm>
#include "limits.h"

using namespace std;

MMKP::MMKP(int n, int m){
		this->n = n;
		this->m = m;
}

MMKP::~MMKP(){
	
}

int MMKP::getLB(){
	int profit = 0;
	for(int i = 0; i < n; i++)
		profit += *min_element(v[i].begin(),v[i].end());
	return profit;
}

int MMKP::getUB(){
	int profit = 0;
	for(int i = 0; i < n; i++)
		profit += *max_element(v[i].begin(),v[i].end());
	return profit;
}

void MMKP::shuffle(){
	for(int i = 0; i < n; i++){
		for(int k = 0; k < m; k++){
			vector<int> v(l[i]);
			for(int j = 0; j < l[i]; j++)
				v[j] = r[i][(j+k) % l[i]][k];
			for(int j = 0; j < l[i]; j++)
				r[i][j][k] = v[j];
		}
	}
}

void MMKP::printESSENCEPrimeInstance(){
	cout << "letting nClasses = " << n << endl;

	cout << "letting nDimensions = " << m  << endl;

	cout << "letting classSize = " << l[0] << endl;

	cout << "letting weight = [" << endl;
	for(int i = 0; i < n; i++){
		cout << "[";
		for(int j = 0; j < l[i]; j++){
			cout << "[";
			for(int k = 0; k < m; k++){
				cout << r[i][j][k];
				if(k < m-1)
					cout << ",";
			}
			cout << "]";
			if(j < l[i]-1)
				cout << ",";
			cout << endl;
		}
		cout << "]";
		if(i < n-1)
			cout << ", ";
		cout << endl;
	}
	cout << "]" << endl;

	cout << "letting capacity = [" << endl;
	for(int i = 0; i < m; i++){
		cout << R[i];
		if(i < m-1)
			cout << ", ";
	}
	cout << "]" << endl;

	cout << "letting profit = [" << endl;
	for(int i = 0; i < n; i++){
		cout << "[";
		for(int j = 0; j < l[i]; j++){
			cout << v[i][j];
			if(j < l[i]-1)
				cout << ",";
		}
		cout << "]";
		if(i < n-1)
			cout << ", ";
		cout << endl;
	}
	cout << "]" << endl;

}

ostream &operator << (ostream &output, MMKP &m)
{
	output << m.n << " " << m.m << endl;
	for(int k = 0; k < m.m; k++)
		cout << m.R[k] << " ";
	cout << endl;
	for(int i = 0; i < m.n; i++){
		cout << i << " " << m.l[i] << endl;
		for(int j = 0; j < m.l[i]; j++){
			cout << m.v[i][j] << " ";
			for(int k = 0; k < m.m; k++)
				cout << m.r[i][j][k] << " ";
			cout << endl;
		}
	}
	
	return output;
}