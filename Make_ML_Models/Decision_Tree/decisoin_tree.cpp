#include <cmath>
#include <cstdlib>
#include <iostream>

#include "decision_tree.h"


DecisionTreeNode::DecisionTreeNode() {
	;
}

void DecisionTreeNode::set_atb_Name(const std::string& atb_name) {
	this -> atb_name = atb_name;
}

std::string DecisionTreeNode::get_atb_Name() {
	return atb_name;
}

void DecisionTreeNode::setType(const std::string& type) {
	this -> type = type;
}

std::string DecisionTreeNode::getType() {
	return type;
}

void DecisionTreeNode::set_Max_T_Val(const std::string& max_T_val) {
	this -> max_T_val = max_T_val;
}

std::string DecisionTreeNode::get_Max_T_Val() {
	return max_T_val;
}

Instance::Instance() {
	;
}

Instance::Instance(const EX& exmp) {
	els = exmp.els;
}

Instance::Instance(
	const std::vector<std::string>& atb_names, const std::vector<std::string>& atb_vals) {
	for (int i = 0; i < atb_names.size(); i++) {
		els[atb_names[i]] = atb_vals[i];
	}
}

std::string Instance::operator[](const std::string& atb_name) const {
	return els.at(atb_name);
}

void Instance::set_atb_Val(const std::string& atb_name, const std::string& atb_val) {
  els[atb_name] = atb_val;
}


std::ostream& operator<<(std::ostream& out, const Instance& inst) {
	for (auto it = inst.els.begin(); it != inst.els.end(); it++) {
		out << it -> first;
		out << ": ";
		out << it -> second;
		out << ", ";
	}
	return out;
}

EX::EX() {
	;
}

EX::EX(
	const std::vector<std::string>& atb_names, const std::vector<std::string>& atb_vals,
	const std::string& target_class) {
	for (int i = 0; i < atb_names.size(); i++) {
		els[atb_names[i]] = atb_vals[i];
	}
	this -> target_class = target_class;
}

std::string EX::get_T_Class() const {
	return target_class;
}

void DecisionTree::add_atb_Info(
	const std::string& atb_name, const std::vector<std::string>& atb_vals) {
	p_vals[atb_name] = atb_vals;
}

void DecisionTree::add_T_Val(const std::vector<std::string>& target_values) {
	this -> target_values = target_values;
}

void DecisionTree::build(const std::vector<EX>& train_data) {
	std::vector<std::string> atb_all;
	for (auto it = p_vals.begin(); it != p_vals.end(); it++) {
		atb_all.push_back(it -> first);
	}
	int nodes = 0;
	build(train_data, root, atb_all, nodes);
}

void DecisionTree::build(std::vector<EX> train_data,
	DecisionTreeNode*& p, std::vector<std::string> check_atb, int& nodes) {

	std::string Max_O_T_Val;

	if (!train_data.empty()) {
		std::map<std::string, int> occ;
		for (auto const& x: train_data) {
			occ[x.get_T_Class()]++;
		}
		int max = -1;
		for (auto const& x: occ) {
			if (x.second > max) {
				max = x.second;
				Max_O_T_Val = x.first;
			}
		}
	}

	//Training 데이터가 있는지 확인하고, 없는 경우 무작위로 대상 클래스를 할당
	if (check_atb.empty()) {
		p = new DecisionTreeNode;++nodes;
		if (train_data.empty()) {
			p -> set_atb_Name(target_values[0]);
		} else {
			p -> set_atb_Name(Max_O_T_Val);
		}
		p -> setType("leaf");
		p -> set_Max_T_Val(p -> get_atb_Name());
		return;
	}

	if (train_data.empty()) {
		p = new DecisionTreeNode;++nodes;
		p -> set_atb_Name(target_values[0]);
		p -> setType("leaf");
		p -> set_Max_T_Val(p -> get_atb_Name());
		return;
	}

	//Target class가 같은지 확인
	bool leaf = true;
	std::string target_class = train_data[0].get_T_Class();
	for (int i = 1; i < train_data.size(); i++) {
		if (train_data[i].get_T_Class() != target_class) {
			leaf = false;
			break;
		}
	}

	if (leaf) {
		p = new DecisionTreeNode;++nodes;
		p -> set_atb_Name(target_class);
		p -> set_Max_T_Val(p -> get_atb_Name());
		p -> setType("leaf");
	} else {
		double Max_gain = -1;
		int Max_index = 0;
		std::vector<double> divide;
		bool is_cont;

		//노드의 attribute 확인
    	for (int i = 0; i < check_atb.size(); i++) {
			if (p_vals[check_atb[i]].size() == 0) {

				std::pair<double, std::vector<double>>temp = C_InfoGain(
					train_data, check_atb[i]);
				double cand_gain = temp.first;
				if (cand_gain > Max_gain) {
					Max_gain = cand_gain;
					Max_index = i;
					is_cont = true;
					divide = temp.second;
				}
			} 
			else {
				double cand_gain = D_InfoGain(train_data, check_atb[i], false);

				if (cand_gain > Max_gain) {
					Max_gain = cand_gain;
					Max_index = i;
					is_cont = false;
				}
			}
		}
		//attribute 확인 후
		std::string atb_name = check_atb[Max_index];
    	check_atb.erase(check_atb.begin() + Max_index);

    	if (is_cont) {
			p = new Continous_DecisionTreeNode;++nodes;
      		p -> setType("continuous");
      		p -> set_atb_Name(atb_name);
			p -> set_Max_T_Val(Max_O_T_Val);

      		Continous_DecisionTreeNode *pp = static_cast<Continous_DecisionTreeNode*>(p);
      		pp -> set_Divide(divide);

      		std::vector<std::vector<EX>> bins;
      		bins.resize(divide.size() + 1);
      		for (int i = 0; i < train_data.size(); i++) {
      			bins[pp -> get_Index(std::stof(train_data[i][atb_name]))].push_back(train_data[i]);
			}

      		for (int i = 0; i <= divide.size(); i++) {
        		build(bins[i], pp -> get_Child_Ptr(i), check_atb, nodes);
      		}

    	} 
		else {
      		D_InfoGain(train_data, atb_name, true);

      		p = new Discrete_DecisionTreeNode;++nodes;
      		p -> setType("discrete");
     		p -> set_atb_Name(atb_name);
			p -> set_Max_T_Val(Max_O_T_Val);

      		Discrete_DecisionTreeNode *pp = static_cast<Discrete_DecisionTreeNode*>(p);

      		std::map<std::string, std::vector<EX>> bins;

      		for (int i = 0; i < train_data.size(); i++) {
      			bins[train_data[i][atb_name]].push_back(train_data[i]);
      		}

			for (int i = 0; i < p_vals[atb_name].size(); i++) {
				build(bins[p_vals[atb_name][i]], (*pp)[p_vals[atb_name][i]], check_atb, nodes);
			}
		}
	}
}

void DecisionTree::prune(const std::vector<EX>& p_data) {
	prune(root, p_data);
}


int DecisionTree::prune(DecisionTreeNode* p, std::vector<EX> p_data) {
	int num_err = 0;
	for (int i = 0; i < p_data.size(); i++) {
	}
	if (p -> getType() == "leaf") {
		return num_err;
	}

	int current_err = num_err;
	int total_child_err = 0;

	if (p -> getType() == "discrete") {

		std::map<std::string, std::vector<EX>> bins;
		for (int i = 0; i < p_data.size(); i++) {
			bins[p_data[i][p -> get_atb_Name()]].push_back(p_data[i]);
		}

		Discrete_DecisionTreeNode* pp = static_cast<Discrete_DecisionTreeNode*>(p);
		std::pair<std::vector<std::string>, std::vector<DecisionTreeNode*>> child_ptrs =
			pp -> get_Child_Ptrs();
		for (int i = 0; i < child_ptrs.first.size(); i++) {
			total_child_err += prune(child_ptrs.second[i], bins[child_ptrs.first[i]]);
		}
	} 
	else {

		Continous_DecisionTreeNode* pp = static_cast<Continous_DecisionTreeNode*>(p);

		std::map<int, std::vector<EX>> bins;
		for (int i = 0; i < p_data.size(); i++) {
			bins[pp -> get_Index(std::stof(p_data[i][p -> get_atb_Name()]))].push_back(p_data[i]);
		}

		std::vector<DecisionTreeNode*> child_ptrs = pp -> get_Child_Ptrs();
		for (int i = 0; i < child_ptrs.size(); i++) {
			total_child_err = prune(child_ptrs[i], bins[i]);
		}
	}
	if (current_err <= total_child_err) {
		p -> setType("leaf");
		p -> set_atb_Name(p -> get_Max_T_Val());
		return current_err;
	} 
	else {
		return total_child_err;
	}
}

double DecisionTree::test(const std::vector<EX>& t_data) {

	int correct = 0, wrong = 0;
	for (int i = 0; i < t_data.size(); i++) {
		Instance temp(t_data[i]);
		if (classify(temp) == t_data[i].get_T_Class()) {
			++correct;
		} else {
			++wrong;
		}
	}
	return ((static_cast<double>(correct) / (wrong + correct)) * 100);
}

void DecisionTree::print() {
	print(root);
}

std::string DecisionTree::classify(const Instance& inst) {
	return classify(inst, root);
}

std::string DecisionTree::classify(const Instance& inst, DecisionTreeNode *p) {
	if (p -> getType() == "leaf") {
		return p -> get_atb_Name();
	} else if (p -> getType() == "continuous") {
		Continous_DecisionTreeNode *pp = static_cast<Continous_DecisionTreeNode*>(p);
		return classify(inst, (*pp)[std::stof((inst[p -> get_atb_Name()]))]);
	} else {
		Discrete_DecisionTreeNode *pp = static_cast<Discrete_DecisionTreeNode*>(p);
		return classify(inst, (*pp)[inst[p -> get_atb_Name()]]);
	}
}


void DecisionTree::print(DecisionTreeNode *p) {
	std::cout << (p -> get_atb_Name()) << "\n";
	if (p -> getType() == "discrete") {
		Discrete_DecisionTreeNode* pp = static_cast<Discrete_DecisionTreeNode*>(p);
		std::vector<DecisionTreeNode*> child_ptrs;
		auto temp = pp -> get_Child_Ptrs();
		for (int i = 0; i < temp.first.size(); i++) {
			std::cout << temp.first[i] << "\n";
			print(temp.second[i]);
		}
	} else if (p -> getType() == "continuous") {
		Continous_DecisionTreeNode* pp = static_cast<Continous_DecisionTreeNode*>(p);
		std::vector<DecisionTreeNode*> child_ptrs;
		child_ptrs = pp -> get_Child_Ptrs();
		for (int i = 0; i < child_ptrs.size(); i++) {
			print(child_ptrs[i]);
		}
	}
}

std::pair<double, std::vector<double> > DecisionTree::C_InfoGain(const std::vector<EX>& els,const std::string& atb_name){
		std::set<std::pair<double,std::string> > C_val_set;
		for(int i=0;i<els.size(); i++)
				C_val_set.insert(make_pair(std::stof(els[i][atb_name]),els[i].get_T_Class()));

		std::vector<std::pair<double,std::string> > C_val_list;
		for(auto it = C_val_set.begin(); it!= C_val_set.end(); it++){
				C_val_list.push_back(*it);
		}

		double pos,info_gain=-1;
		double sum = els.size();

		std::map<std::string, int> entropy_map;
		for(int i=0; i< els.size(); i++)
 		 		entropy_map[els[i].get_T_Class()]++;

 	double entropy_1 = Entropy(entropy_map);

 	std::map<std::string, int> left, right = entropy_map;
 	left[C_val_list[0].second] += 1;
	right[C_val_list[0].second] -= 1;
	double Max_divide, Max_info = -1;
	for(int i=1; i< C_val_list.size(); i++){

		if(C_val_list[i].second != C_val_list[i-1].second) {
			double curr_info = entropy_1 - ((left.size() * Entropy(left) + right.size() * Entropy(right)) / sum);
			if (curr_info > Max_info) {
				Max_info = curr_info;
				Max_divide = (C_val_list[i].first + C_val_list[i - 1].first) / 2;
			}
		}
		right[C_val_list[i].second] -= 1;
		left[C_val_list[i].second] += 1;
	}
	
	std::vector<double> temp;
	temp.push_back(Max_divide);
	return make_pair(Max_info,temp);
}