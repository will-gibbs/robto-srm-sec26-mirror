

Description
- Terminology:
	- Output node = a node that changes the state of a system component
		- Physical or data component
		- Example: motor node
	- Input node = a node that reads and transfers data
		- Does nothing to change the state of a component
		- Example: sensor node
- Context:
	- The system includes several controller nodes, one for each type of task that the robot must do
		- e.g., retrieve ducks, return ducks, restore power to antennae, read colors from antennae
	- Each controller has access to all of the input and output nodes that it needs
		- So one node may be "owned" and used by several controllers
- Problems:
	- How do we keep an output node from being used by more than one controller at the same time?
	- How do we prevent extended delays? (i.e., how do we prevent a controller from having to wait for a long time because another controller is using an output node?)
	- How do we keep a controller from getting confused and trying to accomplish its tasks by using the output nodes that it has access to before it has access to the rest of the output nodes that it needs to accomplish its tasks?
- Solution:
	- Have a proverbial "talking stick"
	- If a controller has the talking stick, it is allowed to use output nodes
	- If a controller does not have the talking stick, it is **not** allowed to use output nodes
	- The **manager** decides which controller to give the talking stick to based on ***task prioritization***
	- The manager may revoke the talking stick any time it deems necessary, according to its own internal logic
- Task Prioritization
	- Each controller uses input nodes and internal logic to identify tasks that it is able to complete from its current position
		- Example: the duck retrieval controller identifies a duck and plans a path to reach and pick up the duck
		- Note: because of the system's modularity, most controllers will only have one task that they will need to complete
	- Each controller decides on only one task that it would complete if it had the talking stick, but it is allowed to change it as other controllers complete other tasks
		- Example: the duck retrieval controller might decide to pick up a specific duck, but in the course of another controller moving to an antenna, because of the rover's new position, the duck retrieval controller decides that it would be more efficient to retrieve another duck instead
	- Each controller publishes its task's values
	- The **manager** calculates the priority of each controller's task. The formula is $Priority = \frac{Point \;Value}{Time \;to \;Complete} \cdot Likelihood \;of \;Success$
		- Point Value - hard coded (ideally located in a database accessible to the entire system)
		- Time to Complete - calculated from known factors, such as distance to the target, time to manipulate the object, etc.
		- Likelihood of Success - derived statistically based on testing and updated consistently
	- The manager sorts all tasks by priority. It then uses this list to make decisions regarding talking stick transfers
		- Note: the manager will likely make decisions based on more than just task priority. Suppose that there are only 30 seconds left in the round. The manager would not want to give the talking stick to a controller that will take 45 seconds to complete its task. Therefore, the manager would need to know the time to complete each task. Suppose now that the round is halfway over, so the manager decides to prioritize mainly on likelihood of success, preferring to win easy points over taking a risk to earn more points. Then the manager would need to know each task's likelihood of success.
	- Example:
		- The manager is deciding which controller to give the talking stick to.
		- The button press antenna controller has identified its task to be restoring power to antenna #1. It looked this task up in the database and found a point value of 15. It determined that it will take 4 seconds to reach the antenna, plus an additional 5 seconds to restore power to the antenna, resulting in a total time of 9 seconds to complete the task. Prior testing revealed an 80% success rate for this task. The controller sent these values to the manager, which used them to calculate a priority of $\frac{15}{9} \cdot 0.8 = 1.33$.
		- The duck retrieval controller has also identified its task to be retrieving the duck that is sitting right next to antenna #1. 5 points are awarded for retrieving and returning each duck to the echo base, so we will say that simply retrieving a duck is worth 2.5 points (this is not consistent with the rules, but we have to assign some point value). The controller calculated a time to complete of 5 seconds (4 to reach the duck and 1 to pick it up) and factored in a success rate of 95%. The controller sent these values to the manager, which used them to calculate a priority of $\frac{5}{5} \cdot 0.95 = 0.95$.
		- The manager has sorted these tasks by priority. It sees that the button press antenna controller's task has a higher priority, so it gives the talking stick to it, and that task is completed.
	- If the system follows this protocol, it will likely complete tasks in the same order each run, but if the calculations are accurate, this order should be close to the most optimal for earning the most points. Additionally, the order may change as testing reveals new likelihoods of success for individual tasks
	- We will likely need to add more variables to the priority calculation. These other variables will reveal themselves during implementation and testing
- Protocol steps
	- On system startup:
		1. The manager starts up first, initializing all of its attributes and methods.
		2. Each controller starts up in sequence:
			1. A controller initializes all of its attributes and methods.
			2. The controller registers itself with the manager, giving the manager a reference to itself that the manager can use when it wants to pass the talking stick to it.
		3. Each controller identifies its task, calculates all of its relevant values, and sends them to the manager via an "update task" method.
		4. The manager collects all task values, calculates task priorities, and sorts the tasks (and by extension the controllers) by priority in descending order.
	- Protocol sequence:
		1. The manager uses task prioritization to decide to give the talking stick to one specific controller.
		2. The manager tells the controller that it is allowed to complete its task, logically passing it the talking stick.
		3. The controller accepts the transfer and begins completing its task.
		4. The controller provides continual feedback to the manager on the progress of its task.
			- At any point, the manager can decide to cancel the task that the controller is trying to perform, essentially revoking the talking stick.
		5. The controller informs the manager that it has completed its task, essentially giving up the talking stick.
		6. The manager uses task prioritization to decide on a new controller to give the talking stick to (note that it may be the same controller).
	- During a run:
		- Each controller is constantly recalculating its task values and updating the manager, which is in turn recalculating task priorities and resorting tasks
- Benefits:
	- Simplified communication between controllers
		- Only one entity—the manager—has to keep track of all controllers
			- **Note**: this follows the *mediator* design pattern
	- Modularity
		- Each controller can focus on completing its tasks without having to keep records of and handle communication with other controllers
	- Maintainability
		- As long as the manager is designed to be able to handle any number of controllers, the removal or addition of a controller requires no changes to any classes
	- Information hiding
		- The only two entities that know which controller has the "talking stick" is the controller that has it and the manager
- Drawbacks/essential points to consider:
	- Each controller must follow the protocol
		- Not a drawback, just a given (but we better make sure it happens)
	- Each controller must have ***clearly-defined and efficient*** logic for:
		- Identifying tasks
		- Calculating task values
		- Completing its task
	- The manager must have clear criteria for when to cancel a task and revoke the talking stick
- Controllers list (tentative):
	- Antennae
		- Button press
		- Pressure pad (duck assault)
		- Keypad
		- Crank
		- Read and display antennae colors
	- Movement
		- Crater navigation
		- Return to starting area
	- Ducks
		- Retrieve ducks
		- Return ducks
	- UAV
		- UAV control
	- Miscellaneous
		- Plant flag
- Design Class Diagram:
![[TSPv2_diagram_2.png]]

