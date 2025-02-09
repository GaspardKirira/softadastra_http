import requests
from sklearn.linear_model import LinearRegression
import numpy as np
import json
import warnings
import urllib3  

warnings.simplefilter("ignore", urllib3.exceptions.InsecureRequestWarning)


# Entraînement d'un modèle de régression
X = np.array([[1000], [2000], [3000], [4000], [5000]])
y = np.array([300, 600, 900, 1200, 1500])

model = LinearRegression()
model.fit(X, y)

# Fonction pour prédire le coût
def predireCout(revenu):
    return model.predict([[revenu]])

# Revenu à prédire
revenu = 3500
predicted_cost = predireCout(revenu)

# Effectuer la requête POST
response = requests.post("https://localhost:8080/predict", json={"revenu": revenu}, verify=False)

# Afficher la réponse
print(f"Réponse API C++: {response.text}")
print(f"Prédiction IA Python: {predicted_cost[0]}")
