import csv
import matplotlib.pyplot as plt
from dataclasses import dataclass
import numpy as np

@dataclass
class TaskProfile:
    name: str
    start_timestamp: float
    end_timestamp: float

    def get_task_id(self) -> int:
        task_id = self.name.split(' ')[0]
        task_id = task_id.split(':')[1]
        return int(task_id)
    
    def is_waiting(self) -> bool:
        return self.name.find('waiting') != -1


all_task_profiles: list[TaskProfile] = []

TEST_NAME = '128MB_4MB_write'
FOLDER_PATH = f'analysis'
CSV_PATH = f'{FOLDER_PATH}/csv/{TEST_NAME}.csv'
IMAGE_PATH = f'{FOLDER_PATH}/png/{TEST_NAME}.png'

with open(CSV_PATH) as csv_file:
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
all_task_profiles = sorted(all_task_profiles, key=lambda profile: profile.get_task_id())

# Prepare data for visualization
names = [profile.name for profile in all_task_profiles if not profile.is_waiting()]
starts = [profile.start_timestamp for profile in all_task_profiles if not profile.is_waiting()]
ends = [profile.end_timestamp for profile in all_task_profiles if not profile.is_waiting()]
durations = np.array(ends) - np.array(starts)

# Create Gantt chart
fig, ax = plt.subplots()
ax.barh(names, durations, left=starts, align='center')
# Add a small dot at the start of each bar

# starts_waiting = [profile.start_timestamp for profile in all_task_profiles if profile.is_waiting()]
# ends_waiting = [profile.end_timestamp for profile in all_task_profiles if profile.is_waiting()]
# names_waiting = [profile.name for profile in all_task_profiles if profile.is_waiting()]
# ax.scatter(starts_waiting, names_waiting, color='blue', s=12, marker='s')  # s is the size of the dots
# ax.scatter(ends_waiting, names_waiting, color='blue', s=12, marker='s')  # s is the size of the dots

ax.set_xlabel('Time')
ax.set_ylabel('Task Name')
ax.set_title('Task Durations')

ax.tick_params(axis='y', labelsize='x-small')

plt.tight_layout()
plt.savefig(IMAGE_PATH, dpi=300)