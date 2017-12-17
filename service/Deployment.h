#ifndef ECO_SERVICE_DEPLOYMENT_H
#define ECO_SERVICE_DEPLOYMENT_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2017-01-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>


namespace eco{ ;
namespace service{ ;


////////////////////////////////////////////////////////////////////////////////
/* service to be deployed on the machines.*/
class Deployment
{
public:
	// get service.
	Service get_service();

	// get machine.
	Machine get_machine();

	// set service name.
	void set_service(IN const char* service_name);

	// set machine mac ip.
	void set_machine(IN const char* mac_ip);

	// set service instance number.
	void set_service_instance_size(IN int size);

	// save data to dops centre.
	void save();

	// remove data from dops centre.
	void remove();

public:
	// implement deployment.
	void deploy();

	// is it deployed.
	bool deployed() const;
};


////////////////////////////////////////////////////////////////////////////////
class DeploymentSet
{
public:
	/*@ command group iterator.*/
	typedef eco::iterator<Deployment> iterator;
	typedef eco::iterator<const Deployment> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ add command group.*/
	void add(IN Deployment&);
	Deployment& add();

	/*@ remove command group.*/
	void erase(IN int);
	iterator erase(IN iterator& it);

	/*@ clear command group.*/
	void clear();

	/*@ get command group set size.*/
	int size() const;
	bool empty() const;

	/*@ access command group by item index.*/
	Deployment& at(IN const int i);
	const Deployment& at(IN const int i) const;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif