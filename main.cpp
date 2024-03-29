#include <iostream>
#include <vector>
#include <cmath>
#include <random>
using namespace std;

struct Derivatives{
    vector<vector<vector<float>>> weights;
    vector<vector<float>> bias;
    Derivatives(vector<vector<vector<float>>> weights_, vector<vector<float>> bias_){
        weights = weights_;
        bias = bias_;
    };
};

class NN {
    private:
    vector<vector<vector<float>>> weights;//layer, float, connections
    vector<vector<float>> bias;
    vector<vector<float>> activation;
    vector<string> activationType;
    default_random_engine generator;
    normal_distribution<float> distribution;
    float relu(float x){
        return max(0.0f,x);
    };
    float reluDerivative(float x){
        if (x > 0) return 1;
        return 0;
    }
    float sigmoid(float x){
        return 1 / (1 + exp(-x));
    }
    float sigmoidDerivative(float x){
        return sigmoid(x)*(1-sigmoid(x));
    }
    float getDerivative(float x, string function){
        if (function == "relu") return reluDerivative(x);
        if (function == "sigmoid") return sigmoidDerivative(x);
        if (function == "linear") return 1;
        cout << "\nwrong function";
        exit(1);
    }
    float getFunction(float x, string function){
        if (function == "relu") return relu(x);
        if (function == "sigmoid") return sigmoid(x);
        if (function == "linear") return x;
        cout << "\nwrong function";
        exit(1);
    }
    vector<float> activationDerivative(vector<float> input, vector<float> activation, string function){
        for (int i = 0; i < input.size(); i++) input[i] = input[i] * getDerivative(activation[i], function);
        return input;
    }
    float dot(vector<float> inputs, vector<float> weight, string function){
        float activation = 0;
        for (float i = 0; i < weight.size(); i++) activation += getFunction(inputs[i], function) * weight[i];
        return activation;
    }
    float cost(float a, float b){
        return (a-b)*(a-b);
    }
    float getDerivative(float prevDerivative, float activation, string function){
        return prevDerivative * getFunction(activation, function);
    }
    vector<float> add(vector<vector<float>> input){
        vector<float> output;
        int t;
        for (int i = 0; i < input[0].size(); i++){
            t = 0;
            for (int j = 0; j < input.size(); j++){
                t += input[j][i];
            }
            output.push_back(t);
        }
        return output;
    }
    Derivatives combine(Derivatives a, Derivatives b){
        for (int i = 0; i < a.weights.size(); i++){
            for (int j = 0; j < a.weights[i].size(); j++){
                for (int k = 0; k < a.weights[i][j].size(); k++){
                    a.weights[i][j][k] += b.weights[i][j][k];
                }
            }
        }
        for (int i = 0; i < a.bias.size(); i++){
            for (int j = 0; j < a.bias[i].size(); j++){
                a.bias[i][j] += b.bias[i][j];
            }
        }
        return a;
    }
    vector<float> multiplyWeight(vector<float> prev, vector<vector<float>> weight){
        vector<float> output;
        for (int i = 0; i < weight[0].size(); i++) output.push_back(0);
        for (int i = 0; i < weight.size(); i++){
            for (int j = 0; j < weight[i].size(); j++){
                output[j] += weight[i][j]*prev[i];
            }
        }
        return output;
    }
    Derivatives divide(Derivatives input, float d){
        for (int i = 0; i < input.weights.size(); i++){
            for (int j = 0; j < input.weights[i].size(); j++){
                for (int k = 0; k < input.weights[i][j].size(); k++){
                    input.weights[i][j][k] /= d;
                }
            }
        }
        for (int i = 0; i < input.bias.size(); i++){
            for (int j = 0; j < input.bias[i].size(); j++){
                input.bias[i][j] /= d;
            }
        }
        return input;
    }
    public:
    void getOutputs(vector<float> X, vector<float> Y){
        for (int i = 0;  i < X.size(); i++){
            cout << predict(X[i]) << ' ' << Y[i] << "\n";
        }
    }
    void initialize(float inputs, float outputs, float mean, float std, string function){
        activationType.push_back("linear");
        activationType.push_back(function);
        distribution = normal_distribution<float> (mean,std);
        vector<vector<float>> newLayer;
        vector<float> curr;
        vector<float> t;
        for (int i = 0; i < outputs; i++) t.push_back(0);
        for (int i = 0; i < inputs; i++) curr.push_back(distribution(generator));
        for (int i = 0; i < outputs; i++) newLayer.push_back(curr);
        weights.push_back(newLayer);
        bias.push_back(t);
    };
    void create_layer(float size, string function){
        activationType.push_back(function);
        vector<vector<float>> newLayer;
        vector<float> connections;
        vector<float> t;
        for (int i = 0; i < size; i++) t.push_back(0);
        for (int i = 0; i < weights[weights.size() - 1].size(); i++) connections.push_back(distribution(generator));
        for (int i = 0; i < size; i++) newLayer.push_back(connections);
        weights.push_back(newLayer);
        bias.push_back(t);
    };
    float predict(float input){
        activation.clear();
        vector<float> prev;//activation of prev layer
        vector<float> curr;
        prev.push_back(input);
        activation.push_back(prev);
        for (int i = 0; i < weights.size(); i++){
            curr.clear();
            for (int j = 0; j < weights[i].size(); j++){
                curr.push_back(dot(prev, weights[i][j], activationType[i]) + bias[i][j]);
            }
            activation.push_back(curr);
            prev = curr;
        }
        return getFunction(prev[0], activationType[activationType.size() - 1]);
    }
    Derivatives backProp(float X, float Y){
        vector<vector<vector<float>>> weightDerivatives;
        vector<vector<float>> layerDerivative;
        vector<vector<float>> biasDerivatives;
        vector<float> prevDerivative;
        vector<float> currDerivative;
        float predicted = predict(X);
        prevDerivative.push_back(getDerivative(activation[activation.size() - 1][0], activationType[activation.size()-2])*(predicted - Y));
        for (int i = weights.size() - 1; i >= 0; i--){
            layerDerivative.clear();
            for (int j = 0; j < weights[i].size(); j++){
                currDerivative.clear();
                for (int k = 0; k < weights[i][j].size(); k++){
                    currDerivative.push_back(getDerivative(prevDerivative[j],  activation[i][k], activationType[i]));
                }
                layerDerivative.push_back(currDerivative);
            }
            biasDerivatives.push_back(prevDerivative);
            weightDerivatives.push_back(layerDerivative);
            prevDerivative = multiplyWeight(prevDerivative, weights[i]);
            prevDerivative = activationDerivative(prevDerivative, activation[i], activationType[i]);
        }

        reverse(weightDerivatives.begin(), weightDerivatives.end());
        reverse(biasDerivatives.begin(), biasDerivatives.end());
        return Derivatives(weightDerivatives, biasDerivatives);
    };
    void fit(vector<float> X, vector<float> Y, float lr, int epochs){
        for (int epoch = 0; epoch < epochs; epoch++){

            Derivatives derivatives = backProp(X[0], Y[0]);
            for (int i = 1; i < X.size(); i++){
                Derivatives currDerivatives = backProp(X[i], Y[i]);
                derivatives = combine(derivatives, currDerivatives);
            }

            derivatives = divide(derivatives, X.size() / lr);
            for (int i = 0; i < weights.size(); i++){
                for (int j = 0; j < weights[i].size(); j++){
                    for (int k = 0; k < weights[i][j].size(); k++){
                        weights[i][j][k] -= derivatives.weights[i][j][k];
                    }
                }
            }
            for (int i = 0; i < bias.size(); i++){
                for (int j = 0; j < bias[i].size(); j++){
                    bias[i][j] -= derivatives.bias[i][j];
                }
            }
        }
        return;
    }
};

int main(){
    //predict test
    vector<float> X;
    vector<float> Y;
    for (float i = 0; i < 5; i++){
        X.push_back(i);
        Y.push_back(i*3 + 2);
//        Y.push_back(-i);
//        Y.push_back(i*i);
    }
    NN model;
    model.initialize(1, 4, 0.5, 0.5, "relu");
    model.create_layer(4, "relu");
    model.create_layer(4, "relu");
    model.create_layer(1, "linear");
    model.fit(X, Y, 0.001, 1000);
    model.getOutputs(X, Y);
    return 0;
}
//b4 = (Observed - Predicted)
//w4 = (Observed - Predicted) * relu(y21)
//b2 = (Observed - Predicted) * w4 * derivativeRelu(y21) * 1
//w2 = (Observed - Predicted) * w4 * derivativeRelu(y21) * 1 * input
