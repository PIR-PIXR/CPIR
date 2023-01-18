import matplotlib.pyplot as plt
from matplotlib.pyplot import text

WY_server = []
LMC_WY_server = []
WY_client = []
LMC_WY_client = []

WY2_server = []
LMC_WY2_server = []
WY2_client = []
LMC_WY2_client = []


#WY only
#case3: t = 1, n = 2^10, m = 2^12 running k_server[] = {2, 3, 3, 4, 4} and d_mapping[] = {3, 4, 5, 6, 7};

#Read data from text file
with open('case3.txt', 'r') as f:
    times = []
    k_and_nm = []
    for line in f:
        try:
            k_and_nm.append(int(line.strip()))
        except ValueError:
            try:
                times.append(float(line.strip()))
            except ValueError:
                pass

#LM-WY*
for i in range(0, 16, 4):
	WY_server.append(times[i])
	i += 1
	WY_client.append(times[i])
	i += 1
	LMC_WY_server.append(times[i])
	i += 1
	LMC_WY_client.append(times[i])

#LM-WY**
for i in range(16, len(times), 4):
	WY2_server.append(times[i])
	i += 1
	WY2_client.append(times[i])
	i += 1
	LMC_WY2_server.append(times[i])
	i += 1
	LMC_WY2_client.append(times[i])

#Server side
x_label = ['3', '4', '5', '6']

bar_width = 0.25

bar_1_x = [i for i in range(len(WY_server))]
bar_2_x = [i + bar_width for i in range(len(WY_server))]

#Server
plt.bar(bar_1_x, WY_server, width=bar_width, bottom = LMC_WY_server, label='WY* (m = 2^10)', fill = False, hatch = '.O')
plt.bar(bar_2_x, WY2_server, width=bar_width, bottom = LMC_WY2_server, label='WY** (m = 2^12)', fill = False, hatch = '.O.O')

plt.bar(bar_1_x, LMC_WY_server, width=bar_width, fill = False, label='LMC-related time', hatch = '///')
plt.bar(bar_2_x, LMC_WY2_server, width=bar_width, fill = False, hatch = '///')

plt.xlabel('d', weight='bold')
plt.ylabel('seconds', weight='bold')
plt.title('Server average time', weight='bold', size = 14)
plt.xticks(bar_2_x, x_label)

# remove top and right spines
plt.gca().spines['right'].set_visible(False)
plt.gca().spines['top'].set_visible(False)

plt.legend(prop={'weight':'bold'})
plt.show()

#Client
plt.bar(bar_1_x, WY_client, width=bar_width, bottom = LMC_WY_client, label='WY* (m = 2^10)', fill = False, hatch = '.O')
plt.bar(bar_2_x, WY2_client, width=bar_width, bottom = LMC_WY2_client, label='WY** (m = 2^12)', fill = False, hatch = '.O.O')

plt.bar(bar_1_x, LMC_WY_client, width=bar_width, fill = False, label='LMC-related time', hatch = '///')
plt.bar(bar_2_x, LMC_WY2_client, width=bar_width, fill = False, hatch = '///')

for i in range(len(WY_client)):
    value = WY_client[i] + LMC_WY_client[i] + 0.5
    plt.text(bar_1_x[i] - 0.05, value, 'WY*', color = 'black', rotation=90, fontweight='bold')
    
    value = WY2_client[i] + LMC_WY2_client[i] + 0.5
    plt.text(bar_2_x[i] - 0.05, value, 'WY**', color = 'black', rotation=90, fontweight='bold')

plt.xlabel('d', weight='bold')
plt.ylabel('seconds', weight='bold')
plt.title('Client time', weight='bold', size = 14)
plt.xticks(bar_2_x, x_label)

# remove top and right spines
plt.gca().spines['right'].set_visible(False)
plt.gca().spines['top'].set_visible(False)

plt.legend(prop={'weight':'bold'})
plt.show()
