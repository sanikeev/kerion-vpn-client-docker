/*
 * Kerio virtual network interface
 * Copyright (C) Kerio Technologies s.r.o.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hdrs.h"
#include "io_read.h"
#include "io_write.h"

#define MAX_COUNT_OF_DEVICES 256

/**
 *	io_dev_iov_total
 */
#undef __FUNC__
#define __FUNC__ "io_dev_iov_total()"
static inline ssize_t
io_dev_iov_total(
	const struct iovec *iv,
	unsigned long count
	)
{
	unsigned long i = 0;
	ssize_t len = 0;

	for (i = 0, len = 0; i < count; i++) {
		len += iv[i].iov_len;
	}

	return len;
}

/**
 *	io_dev_aio_read
 */
#undef __FUNC__
#define __FUNC__ "io_dev_aio_read()"
static ssize_t 
io_dev_aio_read(
	struct kiocb *iocb,
	const struct iovec *iv,
	unsigned long count,
	loff_t pos
	)
{	
	ssize_t len = io_dev_iov_total(iv, count);

#if !defined (USE_BLOCKING_READ)

	//
	//	file is set as blocking ?
	//
	if (!(iocb->ki_filp->f_flags & O_NONBLOCK)) {
		TRACE(LOG_WARN, "%s: this module is compiled with nonblocking file support only.", __FUNC__);
		return -EINVAL;			
	}

#endif // USE_BLOCKING_READ

	if (len < 0) {
		TRACE(LOG_WARN, "%s: io_dev_iov_total(): len=%d", __FUNC__, len);
		return -EINVAL;
	}	

	return io_read_to_iovec((struct iovec *)iv, len);	
}

/**
 *	io_dev_aio_write
 */
#undef __FUNC__
#define __FUNC__ "io_dev_aio_write()"
static ssize_t 
io_dev_aio_write(
	struct kiocb *iocb,
	const struct iovec *iv,
	unsigned long count,
	loff_t pos
	)
{
	ssize_t len = io_dev_iov_total(iv, count);

#if !defined (USE_BLOCKING_READ)

	//
	//	file is set as blocking ?
	//
	if (!(iocb->ki_filp->f_flags & O_NONBLOCK)) {
		TRACE(LOG_WARN, "%s: this module is compiled with nonblocking file support only.", __FUNC__);
		return -EINVAL;			
	}

#endif // USE_BLOCKING_READ

	if (len < 0) {				
		TRACE(LOG_WARN, "%s: io_dev_iov_total(): len=%d", __FUNC__, len);
		return -EINVAL;
	}	

	return io_write_from_iovec((struct iovec *)iv, len);		
}

/**
 *	io_dev_wakeup
 */
#undef __FUNC__
#define __FUNC__ "io_dev_wakeup()"
void
io_dev_wakeup(
	void
	)
{
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_DBG, "%s: trying to wake-up pending io request.", __FUNC__);
	
	wake_up_interruptible(&vnet->io_dev.os_specific.wait_queue);	
}

#if !defined (USE_BLOCKING_READ)

/**
 *	io_dev_poll (select)
 *
 *	POLLIN      Data is available on the file descriptor for reading.
 *
 *	POLLNORM    Same as POLLIN.
 *
 *  POLLPRI     Same as POLLIN.
 *
 *  POLLOUT     Data can be written to the file descriptor without blocking.
 *
 *  POLLERR     This flag is not used in this implementation and is provided
 *              only for source code compatibility.
 *
 *  POLLHUP     The file descriptor was valid before the polling process and
 *              invalid after.  Presumably, this means that the file descrip-
 *              tor was closed sometime during the poll.
 *
 *  POLLNVAL    The corresponding file descriptor is invalid.
 *
 *  POLLRDNORM  Same as POLLIN.
 *
 *  POLLRDBAND  Same as POLLIN.
 *
 *  POLLWRNORM  Same as POLLOUT.
 *
 *  POLLWRBAND  Same as POLLOUT.
 *
 *  POLLMSG     This flag is not used in this implementation and is provided
 *              only for source code compatibility.
 *
 */
#undef __FUNC__
#define __FUNC__ "io_dev_poll()"
static unsigned int 
io_dev_poll(
	struct file *filp,
	poll_table *wait
	)
{	
	unsigned int mask = POLLPRI | POLLERR;
	struct vnet *vnet = vnet_dev();	

	if (vnet->io_dev.canceled) {
		TRACE(LOG_DBG, "%s: io request has been canceled.", __FUNC__);
		return mask;
	}

	do {

		//
		//	We are always writable
		//
		mask = POLLOUT | POLLWRNORM;

		//
		//	only insert into poll table and immediately return
		//
		poll_wait(filp, &vnet->io_dev.os_specific.wait_queue, wait);					

		//
		//	empty queue ?
		//								
		if (queue_is_empty(&vnet->net_dev.queue) == 0) {			
			mask |= POLLIN | POLLRDNORM;
		}				

	} while(0);

	TRACE(LOG_DBG, "%s: writable=%s, readable=%s",
		__FUNC__, 
		((mask & POLLOUT) ? ("yes") : ("no")),
		((mask & POLLIN) ? ("yes") : ("no"))
		);	

	return mask;
}

#endif // USE_BLOCKING_READ

/**
 *	io_dev_ioctl
 */
#undef __FUNC__
#define __FUNC__ "io_dev_ioctl()"
long
io_dev_ioctl(	
	struct file *file,
	unsigned int cmd,
	unsigned long arg
	)
{
	int ret = -EINVAL;
	struct vnet *vnet = vnet_dev();

	switch (cmd) {		

		case KVNET_IOCTL_CANCEL_PENDING_READ:
			{
				TRACE(LOG_INFO, "%s: KVNET_IOCTL_CANCEL_PENDING_READ", __FUNC__);

				//
				//	mark I/O device as canceled
				//
				vnet->io_dev.canceled = 1;

				io_dev_wakeup();

				ret = 0;
			}
			break;

		case KVNET_IOCTL_GET_INFO:
			{
				struct kvnet_info *info = (struct kvnet_info __user *) arg;

				TRACE(LOG_INFO, "%s: KVNET_IOCTL_GET_INFO", __FUNC__);

				if (put_user(KVNET_MAJOR_VERSION, &info->api_version.major)) {
					TRACE(LOG_WARN, "%s: KVNET_IOCTL_GET_INFO: put_user(): segmentation fault.", __FUNC__);
					ret = -EFAULT;
					break;
				}

				if (put_user(KVNET_MINOR_VERSION, &info->api_version.minor)) {
					TRACE(LOG_WARN, "%s: KVNET_IOCTL_GET_INFO: put_user(): segmentation fault.", __FUNC__);
					ret = -EFAULT;
					break;
				}

				if (copy_to_user(&info->hw_addr, vnet->cfg.mac, ETH_ALEN)) {
					TRACE(LOG_WARN, "%s: KVNET_IOCTL_GET_INFO: copy_to_user(): segmentation fault.", __FUNC__);
					ret = -EFAULT;
					break;
				}

				ret = 0;
			}
			break;

		default:
			TRACE(LOG_DBG, "%s: cmd=0x%08X, arg=0x%08X", __FUNC__, cmd, arg);
			break;
	}	

	return ret;
}

/**
 *	io_dev_open
 */
#undef __FUNC__
#define __FUNC__ "io_dev_open()"
static int 
io_dev_open(
	struct inode *inode,
	struct file *filp
	)
{	
	int ret = -EACCES;
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	do {

		//
		//	already opened ?
		//
		if (vnet->io_dev.opened) {
			TRACE(LOG_DBG, "%s: char device \"%s\" already opened.", __FUNC__, KVNET_DEVICE);			
			break;
		}	

		//
		//	add net device
		//
		if ((ret = net_dev_add())) {	
			break;
		}

		vnet->io_dev.opened = 1;
		vnet->io_dev.canceled = 0;

		TRACE(LOG_INFO, "%s: char device \"%s\" opened.", __FUNC__, KVNET_DEVICE);

	} while(0);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return ret;
}

/**
 *	io_dev_close
 */
#undef __FUNC__
#define __FUNC__ "io_dev_close()"
static int 
io_dev_close(
	struct inode *inode,
	struct file *filp
	)
{	
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	//
	//	remove net device
	//
	net_dev_remove();

	vnet->io_dev.opened = 0;	

	TRACE(LOG_INFO, "%s: char device \"%s\" closed.", __FUNC__, KVNET_DEVICE);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return 0;
}

//
//	io_fops
//
static const struct file_operations io_fops = {	
	.owner		= THIS_MODULE,		
#if !defined (USE_BLOCKING_READ)
	.poll		= io_dev_poll,
#endif
	.unlocked_ioctl = io_dev_ioctl,	
	.open		= io_dev_open,
	.release	= io_dev_close,	
	.read		= do_sync_read,
	.aio_read	= io_dev_aio_read,
	.write		= do_sync_write,
	.aio_write	= io_dev_aio_write
};

/**
 *	io_dev_add
 */
#undef __FUNC__
#define __FUNC__ "io_dev_add()"
int
io_dev_add(
	void
	)
{
	dev_t dev;	
	int ret = 0;
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	do {		

		init_waitqueue_head(&vnet->io_dev.os_specific.wait_queue);			

		vnet->io_dev.os_specific.major_number = -1;
		vnet->io_dev.os_specific.minor_number = -1;
		vnet->io_dev.os_specific.class = ERR_PTR(-ENOMEM);
		vnet->io_dev.os_specific.device = ERR_PTR(-ENOMEM);
	
		if ((ret = alloc_chrdev_region(&dev, 0, MAX_COUNT_OF_DEVICES, "kvnet"))) {		
			TRACE(LOG_ERR, "%s: alloc_chrdev_region(): %d", __FUNC__, ret);
			break;
		}

		vnet->io_dev.os_specific.major_number = MAJOR(dev);
		vnet->io_dev.os_specific.minor_number = MINOR(dev);	

		TRACE(LOG_INFO, "%s: device number: major=%d, minor=%d",
			__FUNC__,
			vnet->io_dev.os_specific.major_number,
			vnet->io_dev.os_specific.minor_number
			);

		cdev_init(&vnet->io_dev.os_specific.cdev, &io_fops);		

		vnet->io_dev.os_specific.cdev.owner = THIS_MODULE;

		if ((ret = cdev_add(&vnet->io_dev.os_specific.cdev, dev, MAX_COUNT_OF_DEVICES))) {		
			TRACE(LOG_ERR, "%s: cdev_add(): %d", __FUNC__, ret);
			break;
		}		
		
		vnet->io_dev.added = 1;

		vnet->io_dev.os_specific.class = class_create(THIS_MODULE, "kvnet");

		if (IS_ERR(vnet->io_dev.os_specific.class)) {
			ret = PTR_ERR(vnet->io_dev.os_specific.class);		
			TRACE(LOG_ERR, "%s: class_create(): %d", __FUNC__, ret);		
			break;
		}		

		vnet->io_dev.os_specific.device = 			
#if defined(LINUX_KERNEL_2_6_22_TILL_2_6_26)
			device_create(vnet->io_dev.os_specific.class, NULL, dev, "kvnet");
#elif defined(LINUX_KERNEL_2_6_27)
			device_create_drvdata(vnet->io_dev.os_specific.class, NULL, dev, NULL, "kvnet");
#else
			device_create(vnet->io_dev.os_specific.class, NULL, dev, NULL, "kvnet");
#endif

		if (IS_ERR(vnet->io_dev.os_specific.device)) {
			ret = PTR_ERR(vnet->io_dev.os_specific.device);							
			TRACE(LOG_ERR, "%s: device_create(): %d", __FUNC__, ret);		
			break;
		}		

		TRACE(LOG_INFO, "%s: char device \"%s\" added.", __FUNC__, KVNET_DEVICE);

	} while(0);	

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return ret;
}

/**
 *	io_dev_remove
 */
#undef __FUNC__
#define __FUNC__ "io_dev_remove()"
int
io_dev_remove(
	void
	)
{
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	if (!IS_ERR(vnet->io_dev.os_specific.device)) {

		device_destroy(
			vnet->io_dev.os_specific.class,
			MKDEV(vnet->io_dev.os_specific.major_number, vnet->io_dev.os_specific.minor_number)
			);		

		vnet->io_dev.os_specific.device = ERR_PTR(-ENOMEM);

		TRACE(LOG_INFO, "%s: char device \"%s\" removed.", __FUNC__, KVNET_DEVICE);
	}			

	if (!IS_ERR(vnet->io_dev.os_specific.class)) {
		class_destroy(vnet->io_dev.os_specific.class);
		vnet->io_dev.os_specific.class = ERR_PTR(-ENOMEM);				
	}

	if (vnet->io_dev.added == 1) {
		cdev_del(&vnet->io_dev.os_specific.cdev);
		vnet->io_dev.added = 0;
	}

	if (vnet->io_dev.os_specific.major_number != -1) {		

		unregister_chrdev_region(
			MKDEV(vnet->io_dev.os_specific.major_number, vnet->io_dev.os_specific.minor_number),
			MAX_COUNT_OF_DEVICES
			);

		vnet->io_dev.os_specific.major_number = -1;
		vnet->io_dev.os_specific.minor_number = -1;				
	}	

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return 0;
}

