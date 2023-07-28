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

# create figure and axes
fig, ax = plt.subplots(1, 2, figsize=(14.2, 5))

#Server
ax[0].bar(bar_1_x, WY_server, width=bar_width, bottom = LMC_WY_server, label='WY* (m = $2^{10}$)', fill = False, hatch = '.O')
ax[0].bar(bar_2_x, WY2_server, width=bar_width, bottom = LMC_WY2_server, label='WY** (m = $2^{12}$)', fill = False, hatch = '.O.O')

ax[0].bar(bar_1_x, LMC_WY_server, width=bar_width, fill = False, label='LMC-related time', hatch = '///')
ax[0].bar(bar_2_x, LMC_WY2_server, width=bar_width, fill = False, hatch = '///')

ax[0].set_xlabel('d', weight='bold', size = 15)
ax[0].set_ylabel('seconds', weight='bold', size = 15)
ax[0].set_title('Server average elapsed time', weight='bold', size = 16)
ax[0].set_xticks(bar_2_x, x_label)

# Increase the size of the x-axis tick labels
ax[0].tick_params(axis='x', labelsize=14)

# Increase the size of the y-axis tick labels
ax[0].tick_params(axis='y', labelsize=14)

# remove top and right spines
ax[0].spines['right'].set_visible(False)
ax[0].spines['top'].set_visible(False)

ax[0].legend(prop={'weight':'bold'}, loc=(0.48 ,0.81))


#Client
ax[1].bar(bar_1_x, WY_client, width=bar_width, bottom = LMC_WY_client, label='WY* (m = $2^{10}$)', fill = False, hatch = '.O')
ax[1].bar(bar_2_x, WY2_client, width=bar_width, bottom = LMC_WY2_client, label='WY** (m = $2^{12}$)', fill = False, hatch = '.O.O')

ax[1].bar(bar_1_x, LMC_WY_client, width=bar_width, fill = False, label='LMC-related time', hatch = '///')
ax[1].bar(bar_2_x, LMC_WY2_client, width=bar_width, fill = False, hatch = '///')

for i in range(len(WY_client)):
    value = WY_client[i] + LMC_WY_client[i] + 0.2
    ax[1].text(bar_1_x[i] - 0.05, value, 'WY*', color = 'black', rotation=90, fontweight='bold')

    value = WY2_client[i] + LMC_WY2_client[i] + 0.2
    ax[1].text(bar_2_x[i] - 0.05, value, 'WY**', color = 'black', rotation=90, fontweight='bold')

plt.xlabel('d', weight='bold', size = 15)
ax[1].set_ylabel('seconds', weight='bold', size = 15)
ax[1].set_title('Client elapsed time', weight='bold', size = 16)
ax[1].set_xticks(bar_2_x, x_label)

# remove top and right spines
ax[1].spines['right'].set_visible(False)
ax[1].spines['top'].set_visible(False)

# Increase the size of the x-axis tick labels
ax[1].tick_params(axis='x', labelsize=14)

# Increase the size of the y-axis tick labels
ax[1].tick_params(axis='y', labelsize=14)

ax[1].legend(prop={'weight':'bold'})

plt.savefig('case3.pdf', dpi=300, bbox_inches='tight')
