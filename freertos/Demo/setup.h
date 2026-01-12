#ifndef SETUP_H
#define SETUP_H

#include "FreeRTOS.h"
#include "task.h"
#include "list.h"
#include "semphr.h"
#include <stdbool.h>
#include "logging_stack.h"


/**
 * @file setup.h
 * @brief Periodic Task Layer (PTL) API built on top of FreeRTOS.
 */

#define MAXIMUM_NAME_SIZE 16

/**
 * @brief Task type.
 */
typedef enum {
    PTask,   /**< Periodic task */
    NPTask   /**< Non-periodic task */
} task_type_t;

/**
 * @brief Job execution state.
 */
typedef enum {
    JOB_IDLE,
    JOB_RUNNING
} job_state_t;

/**
 * @brief Overrun handling policy.
 */
typedef enum {
    POLICY_SKIP,      /**< Skip new job if previous not finished */
    POLICY_KILL,      /**< Kill running job and release new one */
    POLICY_CATCH_UP   /**< Release new job immediately, keep cadence */
} overrun_policy_t;

/**
 * @brief PTL task descriptor.
 *
 * This structure describes both configuration and runtime state
 * of a periodic or non-periodic task managed by the PTL.
 */
typedef struct vProcessus {
    uint32_t pid;                         /**< Task identifier */
    char name[MAXIMUM_NAME_SIZE];         /**< Task name */
    TaskFunction_t function;              /**< User job function */
    void *arg;                            /**< User argument */
    UBaseType_t priority;                 /**< FreeRTOS priority */
    task_type_t type;                     /**< Periodic or not */

    /* Periodic parameters */
    TickType_t period;                    /**< Task period */
    TickType_t deadline;                  /**< Relative deadline */
    bool trace_enabled;                   /**< allows disabling logs for some tests */
    overrun_policy_t overrun_policy;      /**< Overrun policy */

    /* Runtime state */
    TaskHandle_t handle;                  /**< FreeRTOS task handle */
    int last_release;                     /**< Last release time, set to int for setting last_release to something < 0 on startup */
    TickType_t abs_deadline;              /**< Absolute deadline */
    uint32_t job_id;                      /**< Job counter */
    volatile job_state_t state;           /**< Job state, we need volatile to prevent strange behaviour ih edge cases */

    SemaphoreHandle_t release_sem;        /**< Release semaphore */
    ListItem_t listItem;                  /**< List linkage */
} vProcessus;

/**
 * @brief Initialise periodic and non-periodic task lists.
 */
void vTaskProcessusInit(List_t *periodic,
                        List_t *nonperiodic);

/**
 * @brief Create a PTL task descriptor.
 *
 * @param name Task name
 * @param function User job function
 * @param arg Function argument
 * @param priority FreeRTOS priority
 * @param is_periodic Periodic flag
 * @param period Task period (ticks)
 * @param deadline Relative deadline (ticks)
 * @param policy Overrun handling policy
 *
 * @return Pointer to created vProcessus
 */
vProcessus* vTaskProcessusCreate(const char *name,
                                 TaskFunction_t function,
                                 void *arg,
                                 UBaseType_t priority,
                                 bool is_periodic,
                                 TickType_t period,
                                 TickType_t deadline,
                                 overrun_policy_t policy,
                                 bool trace_enabled);

/**
 * @brief Insert a task into the appropriate configuration list.
 */
void TaskConfigListPNP_Add(vProcessus *process,
                           List_t *PeriodList,
                           List_t *NonPeriodList);

                           
/**
 * @brief Crée une tâche PTL et l'ajoute automatiquement à la bonne liste.
 *
 * @param name Nom de la tâche
 * @param function Fonction utilisateur
 * @param arg Argument passé à la fonction
 * @param priority Priorité FreeRTOS
 * @param is_periodic Flag indiquant si la tâche est périodique
 * @param period Période (ticks), 0 si non périodique
 * @param deadline Deadline relative (ticks), 0 pour égal à la période
 * @param policy Politique d'overrun (SKIP/KILL/CATCH_UP)
 * @param PeriodList Liste des tâches périodiques
 * @param NonPeriodList Liste des tâches non périodiques
 *
 * @return pointeur vers la vProcessus créée
 */
vProcessus* vCreateAndAddTask(const char *name,
                              TaskFunction_t function,
                              void *arg,
                              UBaseType_t priority,
                              bool is_periodic,
                              bool trace_enabled,
                              TickType_t period,
                              TickType_t deadline,
                              overrun_policy_t policy,
                              List_t *PeriodList,
                              List_t *NonPeriodList);


/**
 * @brief Create FreeRTOS tasks for all periodic PTL tasks.
 */
void vListProcLaunchPerioc(List_t *PeriodicTaskConfigList);

/**
 * @brief Create FreeRTOS tasks for all non-periodic tasks.
 */
void vListProcLaunchNonPerioc(List_t *NonPeriodicTaskConfigList);

/**
 * @brief Start the periodic release manager task.
 */
void vStartPeriodicScheduler(List_t *PeriodicTaskConfigList);

#endif
