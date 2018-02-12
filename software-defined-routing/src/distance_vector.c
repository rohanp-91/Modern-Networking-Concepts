
#include "../include/allinclude.h"

void assignToSelf()
{
	//printf("%s\n", "in assignToSelf");
	router_conn = malloc(sizeof(struct RouterConn));
	LIST_FOREACH(router_conn, &router_list, next)
	{
		if(router_conn->cost == 0)
		{
			self->router_id = router_conn->router_id;
			self->router_port = router_conn->router_port;
			self->data_port = router_conn->data_port;
			self->router_ip_t = router_conn->router_ip_t;
			strcpy(self->router_ip, router_conn->router_ip);

			int timer_fd;
			struct itimerspec timer;
			timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

			timer.it_value.tv_sec = update_interval;
			timer.it_value.tv_nsec = 0;
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_nsec = 0;

			if(timerfd_settime(timer_fd, 0, &timer, NULL) < 0)
				ERROR("Error setting timer value for router!");

			router_conn->timer_val = timer_fd;
			addToList(timer_fd);
		}
	}
	//printf("%s\n", "leaving assignToSelf");
}


void update_cost_matrix()
{
	//printf("%s\n", "BEFORE UPDATE: ");
	//print_cost_mat();

	//make_symmetric();

	int dest, neighbor;
	int self_index = get_index(self->router_id);
	uint16_t cost, mincost, nexthop;
	dest = 0;
	router_conn = malloc(sizeof(struct RouterConn));
	LIST_FOREACH(router_conn, &router_list, next)
	{
		bool flag = FALSE;
		neighbor = 0;
		mincost = INF;
		//nexthop = INF;

		if(dest == self_index) 
		{
			dest++;
			continue;
		}

		// printf("self index = %d\n", self_index);
		// printf("dest = %d\n", dest);
	
		router_conn_temp = malloc(sizeof(struct RouterConn));
		LIST_FOREACH(router_conn_temp, &router_list, next)
		{
			//printf("neighbor = %d\n", neighbor);
			if(router_conn_temp->isNeighbor)
			{
				//printf("neighbor at index: %d\n", neighbor);
				if(cost_mat[neighbor][dest] != INF)
				{
					//printf("setting cost - self index = %d, dest = %d, neighbor = %d\n", self_index, dest, neighbor);
					cost = neighbor_cost[neighbor] + cost_mat[neighbor][dest];
					
					if(cost < neighbor_cost[neighbor] && cost < cost_mat[neighbor][dest])
					 	cost = INF;

					if(cost < mincost)
					{
						mincost = cost;
						nexthop = router_conn_temp->router_id; 
						flag = TRUE;
					}
				}
			}
			neighbor++;
		}

		if(flag)
		{
			router_conn->nexthop = nexthop;
			router_conn->cost = mincost;
			cost_mat[self_index][dest] = mincost;
		}
		dest++;
	}
	//printf("%s\n", "AFTER UPDATE: ");
	//print_cost_mat();

	//printf("ROUTING TABLE: \n");
	//print_routing_table();
}