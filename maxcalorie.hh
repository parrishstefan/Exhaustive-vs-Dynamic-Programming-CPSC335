////////////////////////////////////////////////////////////////////////////////
// maxcalorie.hh
//
// Compute the set of foods that maximizes the calories in foods, within 
// a given maximum weight with the dynamic programming or exhaustive search.
//
///////////////////////////////////////////////////////////////////////////////


#pragma once


#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


// One food item available for purchase.
class FoodItem
{
	//
	public:
		
		//
		FoodItem
		(
			const std::string& description,
			double weight_ounces,
			double calories
		)
			:
			_description(description),
			_weight_ounces(weight_ounces),
			_calories(calories)
		{
			assert(!description.empty());
			assert(weight_ounces > 0);
		}
		
		//
		const std::string& description() const { return _description; }
		double weight() const { return _weight_ounces; }
		double foodCalories() const { return _calories; }
	
	//
	private:
		 int max(int a, int b) {
	return (a > b) ? a : b;
}
		// Human-readable description of the food, e.g. "spicy chicken breast". Must be non-empty.
		std::string _description;
		
		// Food weight, in ounces; Must be positive
		double _weight_ounces;
		
		// Calories; most be non-negative.
		double _calories;
};


// Alias for a vector of shared pointers to FoodItem objects.
typedef std::vector<std::shared_ptr<FoodItem>> FoodVector;


// Load all the valid food items from the CSV database
// Food items that are missing fields, or have invalid values, are skipped.
// Returns nullptr on I/O error.
std::unique_ptr<FoodVector> load_food_database(const std::string& path)
{
	std::unique_ptr<FoodVector> failure(nullptr);
	
	std::ifstream f(path);
	if (!f)
	{
		std::cout << "Failed to load food database; cannot open file: " << path << std::endl;
		return failure;
	}
	
	std::unique_ptr<FoodVector> result(new FoodVector);
	
	size_t line_number = 0;
	for (std::string line; std::getline(f, line); )
	{
		line_number++;
		
		// First line is a header row
		if ( line_number == 1 )
		{
			continue;
		}
		
		std::vector<std::string> fields;
		std::stringstream ss(line);
		
		for (std::string field; std::getline(ss, field, '^'); )
		{
			fields.push_back(field);
		}
		
		if (fields.size() != 3)
		{
			std::cout
				<< "Failed to load food database: Invalid field count at line " << line_number << "; Want 3 but got " << fields.size() << std::endl
				<< "Line: " << line << std::endl
				;
			return failure;
		}
		
		std::string
			descr_field = fields[0],
			weight_ounces_field = fields[1],
			calories_field = fields[2]
			;
		
		auto parse_dbl = [](const std::string& field, double& output)
		{
			std::stringstream ss(field);
			if ( ! ss )
			{
				return false;
			}
			
			ss >> output;
			
			return true;
		};
		
		std::string description(descr_field);
		double weight_ounces, calories;
		if (
			parse_dbl(weight_ounces_field, weight_ounces)
			&& parse_dbl(calories_field, calories)
		)
		{
			result->push_back(
				std::shared_ptr<FoodItem>(
					new FoodItem(
						description,
						weight_ounces,
						calories
					)
				)
			);
		}
	}

	f.close();
	
	return result;
}


// Convenience function to compute the total weight and calories in 
// a FoodVector.
// Provide the FoodVector as the first argument
// The next two arguments will return the weight and calories back to 
// the caller.
void sum_food_vector
(
	const FoodVector& foods,
	double& total_weight,
	double& total_calories
)
{
	total_weight = total_calories = 0;
	for (auto& food : foods)
	{
		total_weight += food->weight();
		total_calories += food->foodCalories();
	}
}


// Convenience function to print out each FoodItem in a FoodVector,
// followed by the total weight and calories of it.
void print_food_vector(const FoodVector& foods)
{
	std::cout << "*** food Vector ***" << std::endl;
	
	if ( foods.size() == 0 )
	{
		std::cout << "[empty food list]" << std::endl;
	}
	else
	{
		for (auto& food : foods)
		{
			std::cout
				<< "Ye olde " << food->description()
				<< " ==> "
				<< "Weight of " << food->weight() << " ounces"
				<< "; calories = " << food->foodCalories()
				<< std::endl
				;
		}
		
		double total_weight, total_calories;
		sum_food_vector(foods, total_weight, total_calories);
		std::cout
			<< "> Grand total weight: " << total_weight << " ounces" << std::endl
			<< "> Grand total calories: " << total_calories
			<< std::endl
			;
	}
}


// Filter the vector source, i.e. create and return a new FoodVector
// containing the subset of the food items in source that match given
// criteria.
// This is intended to:
//	1) filter out food with zero or negative calories that are irrelevant to // our optimization
//	2) limit the size of inputs to the exhaustive search algorithm since it // will probably be slow.
//
// Each food item that is included must have at minimum min_calories and 
// at most max_calories.
//	(i.e., each included food item's calories must be between min_calories
// and max_calories (inclusive).
//
// In addition, the the vector includes only the first total_size food items
// that match these criteria.
std::unique_ptr<FoodVector> filter_food_vector
(
	const FoodVector& source,
	double min_calories,
	double max_calories,
	int total_size
)
{

	if(total_size <= 0) {
		std::cout << "invalid total size\n";
		return nullptr;
	}
	
	std::unique_ptr<FoodVector> newFood(new FoodVector);

	for ( auto & foods : source) {
        if(foods->foodCalories() > 0 && foods->foodCalories() >= min_calories && foods->foodCalories() <= max_calories && newFood->size() < total_size) {
            newFood->push_back(foods);
        }
    }
	
	return newFood;
}

// Compute the optimal set of food items with a exhaustive search algorithm.
// Specifically, among all subsets of food items, return the subset 
// whose weight in ounces fits within the total_weight one can carry and
// whose total calories is greatest.
// To avoid overflow, the size of the food items vector must be less than 64.
std::unique_ptr<FoodVector> exhaustive_max_calories
(
	const FoodVector& foods,
	double total_weight
)
{
	double candidateWeight;
    double candidateCalories;
    double bestCalories;
    double bestWeight;

	const int n = foods.size();
	assert(n < 64);
 	
    
    // 2^(size of foods)
    int nSquared = pow(2, n);
    
    // Optimal vector for foods
    std::unique_ptr<FoodVector> best (new FoodVector);
   
    for (int i = 0; i < nSquared; i++) {
        // Possible vector to compare with
        std::unique_ptr<FoodVector> candidate (new FoodVector);
        for (int j = 0; j < foods.size(); j++) {
            if (((i >> j) & 1) == 1)
                // Adds possible foods to candidate
                candidate->push_back(foods[j]);
        }
        
        // Returns the total weight and calories
        sum_food_vector(*candidate, candidateWeight, candidateCalories);
        sum_food_vector(*best, bestWeight, bestCalories);
        
        // If weight isn't exceeded and optimal calories
        // give candidate foods to best
        if (candidateWeight <= total_weight)
            if (best->empty() || candidateCalories > bestCalories)
                *best = *candidate;
    }
    
    return best;

}

// Compute the optimal set of food items with dynamic programming.
// Specifically, among the food items that fit within a total_weight,
// choose the foods whose calories-per-weight is greatest.
// Repeat until no more food items can be chosen, either because we've 
// run out of food items, or run out of space.
std::unique_ptr<FoodVector> dynamic_max_calories
(
	const FoodVector& foods,
	int total_weight
)

{
	int n = foods.size();
	int W = total_weight;

    std::vector<std::vector<double>> K(n + 1, std::vector<double>(W + 1));
	std::unique_ptr<FoodVector> best(new FoodVector);
      
    // Build table K[][] in bottom up manner
    for(int i = 0; i <= n; i++)
    {
        for(int w = 0; w <= W; w++)
        {
            if (i == 0 || w == 0)
                K[i][w] = 0;
            else if (foods[i - 1]->weight() <= w) {
                K[i][w] = std::max(foods[i - 1]->foodCalories() + K[i - 1][w - foods[i - 1]->weight()], K[i - 1][w]);
		    }
            else {
                K[i][w] = K[i - 1][w];
			}
        }
    }

    int w = total_weight;

    for (int i = n; i > 0; i--) {
        // Either the result comes from the top, K[i-1][w], or from (val[i-1] + K[i-1] [w-wt[i-1]])
        // as in the Knapsack table. If it comes from the latter, the item is included.
        if (K[i][w] == K[i - 1][w])
            continue;   
        else {
            best->push_back(foods[i - 1]);

            // Since this weight is included, its value is deducted.
            w -= foods[i - 1]->weight();
        }
    }

  return best;
}

//
