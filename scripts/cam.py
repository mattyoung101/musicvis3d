from itertools import tee

cam = """CameraPose(glm::vec3(-11.2568, 6.9624, 17.9376), glm::quat(0.9579, -0.0751, -0.2761, -0.0216))
CameraPose(glm::vec3(26.7959, 6.9624, 13.3181), glm::quat(0.9074, -0.0902, 0.4085, 0.0406))
CameraPose(glm::vec3(26.7959, 6.9624, 13.3181), glm::quat(0.9074, -0.0902, 0.4085, 0.0406))
CameraPose(glm::vec3(25.9448, 6.9624, -14.6998), glm::quat(0.3426, -0.0220, 0.9373, 0.0603))
CameraPose(glm::vec3(25.9448, 6.9624, -14.6998), glm::quat(0.3426, -0.0220, 0.9373, 0.0603))
CameraPose(glm::vec3(-9.9373, 6.9624, -15.1438), glm::quat(-0.3448, 0.0280, 0.9352, 0.0759))
CameraPose(glm::vec3(-9.9373, 6.9624, -15.1438), glm::quat(-0.3448, 0.0280, 0.9352, 0.0759))
CameraPose(glm::vec3(-12.9327, -12.1777, -10.1163), glm::quat(-0.4677, -0.1391, 0.8367, -0.2488))
CameraPose(glm::vec3(-12.9327, -12.1777, -10.1163), glm::quat(-0.4677, -0.1391, 0.8367, -0.2488))
CameraPose(glm::vec3(-8.8096, -5.5783, 18.5012), glm::quat(-0.9448, -0.1127, 0.3055, -0.0364))
CameraPose(glm::vec3(-8.8096, -5.5783, 18.5012), glm::quat(-0.9448, -0.1127, 0.3055, -0.0364))
""".strip().splitlines()

# doesn't work
for a, b in zip(cam[::2], cam[1::2]):

# # awful fucking code whatever
# for i in range(len(cam) - 1):
#     a = cam[i]
#     b = cam[i + 1]
    print(f"CameraAnimation(\n    {a},\n    {b},\n    10.f\n),")
