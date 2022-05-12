
# Smart Kitchen MQTT Specifications

**Server**:  192.168.0.100
**Port**:1883

# Topics

## "mood/state"

**Description**: Turn ON/OFF the mood service or select the desired mood
**Payload**: string
**Payload format**: "status"
**Example**: "0"
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description    | Values          | Required |
|--------|--------|----------------|-----------------------------------|----------|
| status | string | Controller for the led moods service. Turns ON/OFF or specifies the required mood| 0-OFF <br> 1-ON <br> 2-Negative mood<br> 3-Medium negative <br> 4-Neutral <br> 5-Medium positive<br> 6-Positive mood | true     |

Note: The mood service needs to be On before any usage

## "mood/state/status"

**Description**: Control topic. Announces the status of the mood service
**Payload**: string
**Payload format**: "status"
**Example**: "0"
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description               | Values                              | Required |
|--------|--------|---------------------------|-------------------------------------|----------|
| status | string | Status of the mood service| 0-OFF <br> 1-ON <br> 2-Negative mood<br> 3-Medium negative <br> 4-Neutral <br> 5-Medium positive<br> 6-Positive mood | true     |

## "mood/color/*"

### *from 1 to 6 each representing one mood service status in which:

|Value  | Description   |
|-------|---------------|
|1      |ON             |
|2      |Negative Mood  |
|3      |Medium Negative|
|4      |Neutral        |
|5      |Medium Positive|
|6      |Positive Mood  |

**Description**: Topic for controlling the color of each mood state
**Payload**: string
**Payload format**: “R,G,B”
**Example**: “255,255,255”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                       | Values                         | Required |
|--------|--------|-----------------------------------|--------------------------------|----------|
| color  | string | RGB string with the required color| Any value from 0 to 255 | true     |


## "mood/color/*/status"

### *from 1 to 6 each representing one mood service status in which:

|Value  | Description   |
|-------|---------------|
|1      |ON             |
|2      |Negative Mood  |
|3      |Medium Negative|
|4      |Neutral        |
|5      |Medium Positive|
|6      |Positive Mood  |

**Description**: Control topic, announces the color of the respective mood state
**Payload**: string
**Payload format**: “R,G,B”
**Example**: “255,255,255”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                       | Values                         | Required |
|--------|--------|-----------------------------------|--------------------------------|----------|
| color  | string | RGB string with the required color| Any value from 0 to 255 | true     |

## "mood/section"

**Description**: Controls the closet sections on the main led strip
**Payload**: string
**Payload format**: “section”
**Example**: “3”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                       | Values                         | Required |
|--------|--------|-----------------------------------|--------------------------------|----------|
| section  | string | Section of the main led strip to be turned ON| 0-OFF<br>1- Closet 1<br>2- Closet 2<br>3-Closet 3<br>4-Closet 4<br>5-Closet 5<br>6-Closet 6 | true     |

## "mood/section/status"

**Description**: Anounnces the closet section that is being used
**Payload**: string
**Payload format**: “section”
**Example**: “3”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                       | Values                         | Required |
|--------|--------|-----------------------------------|--------------------------------|----------|
| section  | string | Section of the main led strip to be turned ON| 0-OFF<br>1- Closet 1<br>2- Closet 2<br>3-Closet 3<br>4-Closet 4<br>5-Closet 5<br>6-Closet 6 | true     |

## "mood/section/color"

**Description**: Topic to define the color of the main led strip’s sections
**Payload**: string
**Payload format**: “R,G,B”
**Example**: “255,255,255”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                | Values                  | Required |
|--------|--------|----------------------------|-------------------------|----------|
| color  | string | RGB string separated by ‘,’| Any value from 0 to 255 | true     |


## "mood/section/color/status"

**Description**: Topic to publish the color of the main led strip’s sections
**Payload**: string
**Payload format**: “R,G,B”
**Example**: “255,255,255”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                | Values                  | Required |
|--------|--------|----------------------------|-------------------------|----------|
| color  | string | RGB string separated by ‘,’| Any value from 0 to 255 | true     |

## "shelf1"

**Description**: Turn ON one section of the Nº1 shelf
**Payload**: string
**Payload format**: “section”
**Example**: “3”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                | Values                  | Required |
|--------|--------|----------------------------|-------------------------|----------|
| section  | string | Section of led to be turned ON| 0-OFF<br>
1-Wine<br>
2-Gin<br>
3-Cookies<br>
4-Apples<br>
| true     |

## "shelf1/staus"

**Description**:Announces which section of the Nº1 shelf is ON
**Payload**: string
**Payload format**: “section”
**Example**: “3”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                | Values                  | Required |
|--------|--------|----------------------------|-------------------------|----------|
| section  | string | Section of led to be turned ON| 0-OFF<br>
1-Wine<br>
2-Gin<br>
3-Cookies<br>
4-Apples<br>
| true     |

## "shelf2"

**Description**: Turn ON one section of the Nº2 shelf
**Payload**: string
**Payload format**: “section”
**Example**: “3”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                | Values                  | Required |
|--------|--------|----------------------------|-------------------------|----------|
| section  | string | Section of led to be turned ON| 0-OFF<br>
1-Pasta<br>
2-Ketchup<br>
3-Rice<br>
4-Water<br>
| true     |

## "shelf2/staus"

**Description**:Announces which section of the Nº2 shelf is ON
**Payload**: string
**Payload format**: “section”
**Example**: “3”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                | Values                  | Required |
|--------|--------|----------------------------|-------------------------|----------|
| section  | string | Section of led to be turned ON| 0-OFF<br>
1-Pasta<br>
2-Ketchup<br>
3-Rice<br>
4-Water<br>
| true     |

## "shelf/color/*"

### *from  0 to 7:

|Value  | Description   |
|-------|---------------|
|0      |Wine section   |
|1      |Gin section    |
|2      |Cookies section|
|3      |Apples section |
|4      |Pasta section  |
|5      |Ketchup section|
|6      |Rice section   |
|7      |Water section  |

**Description**: Changes the color of a section of the shelves
**Payload**: string
**Payload format**: “R,G,B”
**Example**: “255,255,255”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                       | Values                         | Required |
|--------|--------|-----------------------------------|--------------------------------|----------|
| color  | string | RGB string with the required color| Any value from 0 to 255 | true     |

## "shelf/color/*/status"

### *from  0 to 7:

|Value  | Description   |
|-------|---------------|
|0      |Wine section   |
|1      |Gin section    |
|2      |Cookies section|
|3      |Apples section |
|4      |Pasta section  |
|5      |Ketchup section|
|6      |Rice section   |
|7      |Water section  |

**Description**: Announces the color of the respective shelf section
**Payload**: string
**Payload format**: “R,G,B”
**Example**: “255,255,255”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                       | Values                         | Required |
|--------|--------|-----------------------------------|--------------------------------|----------|
| color  | string | RGB string with the required color| Any value from 0 to 255 | true     |

## " automatic_cabinet_door/controls"

**Description**: Commands for the cabinet doors
**Payload**: string
**Payload format**: “command”
**Example**: “AUTOMATIC_MODE”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                   | Values                         | Required |
|--------|--------|-------------------------------|--------------------------------|----------|
| command| string |Automatic cabinet door commands|AUTOMATIC_MODE,MANUAL_MODE,OPEN_MODE,OPEN,CLOSE
 | true     |

## " voice_control/status"

**Description**: Signals the voice recognition command that is being used
**Payload**: string
**Payload format**: “command”
**Example**: “Wine”
| QoS    | 0    |
|--------|------|
| Retain | true |

| Name   | Type   | Description                  | Values        | Required |
|--------|--------|------------------------------|---------------|----------|
| command| string |Command that is being executed|See table below| true     |


|Value          | Description |
|---------------|----------------|
|ON             | Voice control is avaliable |
|WAKE_UP_KITCHEN|Activates the voice controlled kitchen|
|SLEEP_KITCHEN  |Deactivated the voice controlled kitchen|
|AUTOMATIC_MODE |Cabinet door opens on approach and closes when moving away |
|MANUAL_MODE    |Cabinet door opens on approach and after the voice command “open” |
|OPEN_MODE      |Cabinet door allows the cabinet to stay continuously open until the voice command “close” |
|OPEN           |Voice command “open” |
|CLOSE          |Voice command “close” |
|LED_MOODS_ON   |Truns ON the led mood service |
|LED_MOODS_OFF  |Turns OFF the led mood service |
|HAPPY MOOD     |Sets happy mood |
|SAD MOOD       |Sets neutral mood |
|NEUTRAL MOOD   |Sets mid-happy mood |
|NEUTRAL HAPPY  |Sets mid-sad mood |
|NEUTRAL SAD    |Foods from the closet nº4 |
|Wine           |Foods from the closet nº4 |
|Gin            |Foods from the closet nº4 |
|Cookies        |Foods from the closet nº4 |
|Apples         |Foods from the closet nº4 |
|Pasta          |Foods from the closet nº4 |
|Ketchup        |Foods from the closet nº4 |
|Rice           |Foods from the closet nº4 |
|Water          |Foods from the closet nº4 |
|Bread          |Foods from the closet nº1 |
|Milk           |Foods from the closet nº2 |
|Pans           |Foods from the closet nº3 |
|Cups           |Foods from the closet nº5 |
|Plates         |Foods from the closet nº6 |
