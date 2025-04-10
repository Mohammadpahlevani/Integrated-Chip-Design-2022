# Cell Placement Optimization

This project focuses on optimizing the placement of cells using two algorithms: a **Greedy algorithm** and **Simulated Annealing (SA)**. The main goal is to minimize the cost function of the layout while ensuring a legal (non-overlapping) placement.

---

## 🚀 Project Phases

### 🟢 Phase 1: Greedy Algorithm

- Two cells are randomly selected and temporarily swapped.
- If the swap **reduces the cost function**, the change is accepted.
- Otherwise, the swap is reverted and the layout returns to the previous state.
- This process continues for a predefined number of iterations.

### 🔥 Phase 2: Simulated Annealing (SA)

- Starting from the current layout (initial or greedy-based), SA is applied.
- Unlike the greedy algorithm, SA **allows occasional acceptance of worse moves** to escape local minima.
- The final placement is evaluated and compared with:
  - The initial (default) placement
  - The greedy placement result

---

## ✅ Final Validation

After running the optimization:

- The layout is checked to ensure **no two cells overlap**.
- For each row, the farthest-right cell is located.
- If any overlap is detected, cells are placed sequentially to ensure a **legal placement**.
