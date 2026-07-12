import requests
import time 
import matplotlib.pyplot as plt
import scipy as sp
import csv
import os

# need all the above libraries for plotting 
file = open("data_log.csv", "w", newline = "")
writer = csv.writer(file)
writer.writerow(["time", "temp", "humidity", "motorspeed"])
plt.ion()
fig, (ax, ax1, ax2) = plt.subplots(3, 1, sharex=True)
# this creates three plots, stacked ontop of ach other. as suggested by three rows (3) and one column (1). they each share the same x axis which is time. this makes sense for the project as i want to see how tempearture and humidity change with time, and the corresponding motor speed for that tempearture 

timedataarray = []
motorspeeddataarray = []
humiditydataarray = []
temperaturedataarray = []
# the above are just the empty arrays needed to store data 

print(os.path.abspath("data_log.csv"))

while True:
    

    try:
        r = requests.get("http://192.168.0.229/data", timeout=5 ) # if we dont get a response from the esp32 in 5 seconds give up.  
        data = r.json()
        # ping the esp32 and ask for data, also put the data that we get in json form, well, try 
        print(data)
        temperaturedata = data["temp"]
        humiditydata = data["humidity"]
        motorspeeddata = data["motorspeed"]
        # also try to get each individual piece of data from the json into seperate data, e.g. tempearture data and humidity data etc. 
        timedata = int(time.time()) # does not rely on esp32 time rather python time in seonds, juist sequential counting. much nmore smoother 
        timedataarray.append(timedata)
        humiditydataarray.append(humiditydata)
        motorspeeddataarray.append(motorspeeddata)   
        temperaturedataarray.append(temperaturedata)
        writer.writerow([timedata, temperaturedata, humiditydata, motorspeeddata])
        file.flush()
        print(timedata, temperaturedata, humiditydata, motorspeeddata, "have all been logged")
        # append all to its correspondign array 
    except requests.exceptions.RequestException: # this is part of the requests module, any network error is included in this e,g, packet lost timing missmatches etc. 
        print("could not connect due to network errors, waiting......")

    if len(temperaturedataarray) > 30: # i want 30 data points at any given isntant, that way the graph can keep moving and not lag as the data is live. 
        temperaturedataarray.pop(0)
        humiditydataarray.pop(0)
        motorspeeddataarray.pop(0)
        timedataarray.pop(0)
        # so remove the first data point of each array when it gets above 30 points 

    ax.clear()
    ax1.clear()
    ax2.clear()
    # clear all axis for plotting because we plot new data points every iteration
    ax.plot(timedataarray, temperaturedataarray, 'r-', label="temperature")
    ax1.plot(timedataarray, motorspeeddataarray, 'b--', label="motor speed")
    ax2.plot(timedataarray, humiditydataarray, 'g--', label="humidity")
    # on the three seperate figures that we have, plot each data array against time with a differet colour adn label it 
    ax.legend() # add a legend for readability
    ax.set_ylabel("temperature in celcius")
    ax1.set_ylabel("motor speed")
    ax2.set_ylabel("humidity %")
    # set thje labels
    plt.pause(0.2)
    # pause plotting before the next iteration. 
