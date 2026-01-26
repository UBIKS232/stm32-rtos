
```mermaid
graph LR
    A[App Task A] -->|调用 vTaskDelay()<br/>或 SysTick 到期| B(Request PendSV)
    B --> C{是否有高优先级中断?}
    C -- Yes --> D[挂起 PendSV<br/>继续处理中断]
    C -- No --> E[Enter PendSV Handler]
    D -->|中断退出后| E
    E --> F[Save Context of Task A]
    F --> G[vTaskSwitchContext()<br/>Select Task B]
    G --> H[Restore Context of Task B]
    H --> I[Return to Thread Mode]
    I --> J[App Task B Running]
```