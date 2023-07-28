import matplotlib.pyplot as plt
from matplotlib.pyplot import text

CKGS_server = []
LMC_CKGS_server = []
CKGS_client = []
LMC_CKGS_client = []

WY_server = []
LMC_WY_server = []
WY_client = []
LMC_WY_client = []

BE_server = []
LMC_BE_server = []
BE_client = []
LMC_BE_client = []

#case2: k = 2, t = 1, m = 2^10: running n (256, 1024, 4096)

#Read data from text file
with open('case2.txt', 'r') as f:
    times = []
    m_and_n = []
    for line in f:
        try:
            m_and_n.append(int(line.strip()))
        except ValueError:
            try:
                times.append(float(line.strip()))
            except ValueError:
                pass

#LM-CKGS
for i in range(0, 12, 4):
	CKGS_server.append(times[i])
	i += 1
	CKGS_client.append(times[i])
	i += 1
	LMC_CKGS_server.append(times[i])
	i += 1
	LMC_CKGS_client.append(times[i])
#LM-WY
for i in range(12, 24, 4):
	WY_server.append(times[i])
	i += 1
	WY_client.append(times[i])
	i += 1
	LMC_WY_server.append(times[i])
	i += 1
	LMC_WY_client.append(times[i])
#LM-BE
for i in range(24, 36, 4):
	BE_server.append(times[i])
	i += 1
	BE_client.append(times[i])
	i += 1
	LMC_BE_server.append(times[i])
	i += 1
	LMC_BE_client.append(times[i])

#Server side
x_label = ['$2^{8}$', '$2^{10}$', '$2^{12}$']

bar_width = 0.25

bar_1_x = [i for i in range(len(CKGS_server))]
bar_2_x = [i + bar_width for i in range(len(WY_server))]
bar_3_x = [i + bar_width*2 for i in range(len(BE_server))]

# create figure and axes
fig, ax = plt.subplots(1, 2, figsize=(14.2, 5))

#Server
ax[0].bar(bar_1_x, CKGS_server, width=bar_width, bottom = LMC_CKGS_server, label='2-CKGS', color = 'black')
ax[0].bar(bar_2_x, WY_server, width=bar_width, label='WY', bottom = LMC_WY_server, fill = False, hatch = '.O.O')
ax[0].bar(bar_3_x, BE_server, width=bar_width, label='BE', bottom = LMC_BE_server, fill = False, hatch = '...')

ax[0].bar(bar_1_x, LMC_CKGS_server, width=bar_width, label='LMC-related time', fill = False, hatch = '///')
ax[0].bar(bar_2_x, LMC_WY_server, width=bar_width, fill = False, hatch = '///')
ax[0].bar(bar_3_x, LMC_BE_server, width=bar_width, fill = False, hatch = '///')

for i in range(len(CKGS_server)):
    value = CKGS_server[i] + LMC_CKGS_server[i] + 1
    ax[0].text(bar_1_x[i] - 0.05, value, '2-CKGS', color = 'black', rotation=90, fontweight='bold')

    value = WY_server[i] + LMC_WY_server[i] + 1
    ax[0].text(bar_2_x[i] - 0.05, value, 'WY', color = 'black', rotation=90, fontweight='bold')

    value = BE_server[i] + LMC_BE_server[i] + 1
    ax[0].text(bar_3_x[i] - 0.05, value, 'BE', color = 'black', rotation=90, fontweight='bold')

ax[0].set_xlabel('n', weight='bold', size = 15)
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

ax[0].legend(prop={'weight':'bold'})


#Client
ax[1].bar(bar_1_x, CKGS_client, width=bar_width, bottom = LMC_CKGS_client, label='2-CKGS', color = 'black')
ax[1].bar(bar_2_x, WY_client, width=bar_width, label='WY', bottom = LMC_WY_client, fill = False, hatch = '.O.O')
ax[1].bar(bar_3_x, BE_client, width=bar_width, label='BE', bottom = LMC_BE_client, fill = False, hatch = '...')

ax[1].bar(bar_1_x, LMC_CKGS_client, width=bar_width, label='LMC-related time', fill = False, hatch = '///')
ax[1].bar(bar_2_x, LMC_WY_client, width=bar_width, fill = False, hatch = '///')
ax[1].bar(bar_3_x, LMC_BE_client, width=bar_width, fill = False, hatch = '///')

for i in range(len(CKGS_server)):
    value = CKGS_client[i] + LMC_CKGS_client[i] + 0.5
    ax[1].text(bar_1_x[i] - 0.05, value, '2-CKGS', color = 'black', rotation=90, fontweight='bold')

    value = WY_client[i] + LMC_WY_client[i] + 0.5
    ax[1].text(bar_2_x[i] - 0.05, value, 'WY', color = 'black', rotation=90, fontweight='bold')

    value = BE_client[i] + LMC_BE_client[i] + 0.5
    ax[1].text(bar_3_x[i] - 0.05, value, 'BE', color = 'black', rotation=90, fontweight='bold')

ax[1].set_xlabel('n', weight='bold', size = 15)
ax[1].set_ylabel('seconds', weight='bold', size = 15)
ax[1].set_title('Client elapsed time', weight='bold', size = 16)
ax[1].set_xticks(bar_2_x, x_label)

# Increase the size of the x-axis tick labels
ax[1].tick_params(axis='x', labelsize=14)

# Increase the size of the y-axis tick labels
ax[1].tick_params(axis='y', labelsize=14)

# remove top and right spines
ax[1].spines['right'].set_visible(False)
ax[1].spines['top'].set_visible(False)

ax[1].legend(prop={'weight':'bold'})

plt.savefig('case2.pdf', dpi=300, bbox_inches='tight')
