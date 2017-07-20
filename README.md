# Autonomous Bottle Sorting Robot Project
This robot was designed to sort bottles of four different categories:
 * Transparent Water Bottle with a Cap
 * Transparent Water Bottle without a Cap
 * Yoplait Yogurt bottle with a Cap
 * Yoplait yogurt bottle without a cap

To sort the bottles, all the bottles are placed in a loading mechanism. The start button is pressed and the bottles are placed one by one onto a sorting tube. A break beam sensor passing through the body of the bottle is used to classify whether the bottle is transparent or not. Proximity sensors strategically placed at both ends of the bottle to sense for a cap. If nothing is detected at either one of the ends of the bottle, this means that there is no cap on the bottle. Otherwise, there is a cap on the bottle. After this a motor rotates a platform to place the correct bin underneath the sorting tube.

The code execute by the robot can be found in `main.c`.
