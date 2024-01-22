import csv
import matplotlib.pyplot as plt
from dataclasses import dataclass
import numpy as np

@dataclass
class TaskProfile:
    name: str
    start_timestamp: float
    end_timestamp: float

all_task_profiles: list[TaskProfile] = []

with open('profiling_write.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for i, row in enumerate(csv_reader):
        if i == 0: all_start_timestamp = row[1]

        task_profile = TaskProfile(
            name=row[0],
            start_timestamp=float(row[1]) - float(all_start_timestamp),
            end_timestamp=float(row[2]) - float(all_start_timestamp)
        )
        all_task_profiles.append(task_profile)

# Sort all_task_profiles by start_timestamp
all_task_profiles = sorted(all_task_profiles, key=lambda profile: profile.start_timestamp)

# Prepare data for visualization
names = [profile.name for profile in all_task_profiles]
starts = [profile.start_timestamp for profile in all_task_profiles]
ends = [profile.end_timestamp for profile in all_task_profiles]
durations = np.array(ends) - np.array(starts)

# Create Gantt chart
fig, ax = plt.subplots()
ax.barh(names, durations, left=starts, align='center')

ax.set_xlabel('Time')
ax.set_ylabel('Task Name')
ax.set_title('Task Durations')
plt.tight_layout()
plt.savefig('profiling_write.png', dpi=300)