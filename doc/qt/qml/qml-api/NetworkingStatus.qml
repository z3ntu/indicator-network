import QtQuick 2.0

/*!

\qmltype NetworkingStatus
\ingroup connectivity
\brief

ActionManager exports the application actions to the external components.
*/

Item {

        /*!
           \qmlproperty int  NetworkingStatus::globalContext

           The globalContext of the Application.

           \note Setting the ActionContext::active on the global context has no effect;
         */
        property int globalContext

//    /*!
//       \qmlproperty ActionContext  ActionManager::globalContext

//       The globalContext of the Application.

//       \note Setting the ActionContext::active on the global context has no effect;
//     */
//    property ActionContext globalContext

//    /*!
//      \qmlproperty list<Action> ActionManager::actions
//      \default
//      List of Actions in manager's globalContext.

//      This is the default property of ActionManager.
//     */
//    property list<Action> actions

//    /*!
//      \qmlproperty list<ActionContext> ActionManager::localContexts

//      List of localContexts.
//     */
//    property list<ActionContext> localContexts


//    /*!
//     this is a shorthand for
//     \qml
//        manager.globalContext.addAction(action);
//     \endqml

//     \sa ActionContext::addAction()
//     */
//    function addAction(action) {}

//    /*!
//     this is a shorthand for
//     \qml
//        manager.globalContext.removeAction(action);
//     \endqml

//     \sa ActionContext::removeAction()
//     */
//    function removeAction(action) {}

//    /*!
//     Adds a local context.

//     Calling this function multiple times with the same context
//     does not have any side effects; the context gets added only once.
//    */
//    function addLocalContext(context) {}

//    /*!
//     Removes a local context.

//     Calling this function multiple times with the same context
//     does not have any side effects; the context gets removed only if
//     it was previously added with addLocalContext().
//    */
//    function removeLocalContext(context) {}

}
